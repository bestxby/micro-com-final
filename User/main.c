#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "lcd.h"
#include "ai_detect.h"
#include "anomaly_log.h"
#include "game.h"
#include "bh1750.h"
#include "sd.h"
#include "tm1637.h"
#include "sr04.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * 全局变量 (传感器 / AI / 系统状态)
 * ============================================================ */
volatile float test_aht20_temp   = 0.0f;
volatile float test_aht20_humi   = 0.0f;

volatile uint8_t test_aht20_init_status  = 1;

volatile uint8_t aht20_healthy  = 1;

uint8_t aht20_fail_cnt  = 0;

/* BH1750 光电传感器与 SD 存储卡状态变量 */
volatile float test_bh1750_lux      = 0.0f;
volatile uint8_t bh1750_healthy     = 1;
volatile uint8_t sd_healthy         = 1;
uint32_t log_sector_cursor          = 1000;
uint8_t log_buffer[512]             = {0};
uint16_t log_buffer_index           = 0;

/* 超声波测距与防盗监控状态变量 */
volatile float test_ultrasonic_dist = 0.0f;
volatile uint8_t ultrasonic_healthy = 1;
volatile uint8_t security_alert_mode = 0; /* 0=Disarmed, 1=Armed */
volatile float distance_history[24] = {0.0f};

AI_Detector     my_detector;
AnomalyLogBuffer my_log_buffer;
volatile float  test_filtered_temp  = 0.0f;
volatile uint8_t current_ai_state   = AI_STATE_LEARNING;

float dev_history[24] = {0.0f};    /* 24 个采样点 → 偏差折线图 */

volatile uint32_t system_uptime_s = 0;
volatile uint8_t  system_mode     = 0;    /* 0=正常, 1=异常 */
volatile uint8_t  current_page    = 0;    /* 0=实时监测, 1=AI自适应, 2=历史日志, 3=游戏 */
volatile uint8_t  last_page       = 99;

/* ============================================================
 * 布局常量 (480×320 横屏适配布局)
 * ============================================================ */
#define TOP_BAR_H      24
#define SEP_Y1         25
#define TITLE_Y        28
#define SEP_Y2         46
#define CONTENT_Y      52
#define BOTTOM_Y       296
#define PAGE_DOT_Y     308

/* ============================================================
 * 内部辅助与 HUD 科技感绘图函数
 * ============================================================ */
void Delay(__IO uint32_t nCount) {
    for (; nCount != 0; nCount--);
}

/* 绘制四角科技感折角边框 */
static void Draw_CornerBrackets(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t len, uint16_t color) {
    // 左上角
    LCD_DrawLine(x, y, x + len, y, color);
    LCD_DrawLine(x, y, x, y + len, color);
    // 右上角
    LCD_DrawLine(x + w - 1 - len, y, x + w - 1, y, color);
    LCD_DrawLine(x + w - 1, y, x + w - 1, y + len, color);
    // 左下角
    LCD_DrawLine(x, y + h - 1 - len, x, y + h - 1, color);
    LCD_DrawLine(x, y + h - 1, x + len, y + h - 1, color);
    // 右下角
    LCD_DrawLine(x + w - 1 - len, y + h - 1, x + w - 1, y + h - 1, color);
    LCD_DrawLine(x + w - 1, y + h - 1 - len, x + w - 1, y + h - 1, color);
}

/* 绘制像素温度计图标 */
static void Draw_TempIcon(uint16_t x, uint16_t y, uint16_t color) {
    LCD_DrawLine(x + 2, y, x + 4, y, color);
    LCD_DrawLine(x + 2, y, x + 2, y + 8, color);
    LCD_DrawLine(x + 4, y, x + 4, y + 8, color);
    LCD_DrawCircle(x + 3, y + 11, 3, color);
    LCD_FillCircle(x + 3, y + 11, 2, theme_red);
    LCD_DrawLine(x + 3, y + 3, x + 3, y + 9, theme_red);
}

/* 绘制像素水滴图标 */
static void Draw_DropIcon(uint16_t x, uint16_t y, uint16_t color) {
    LCD_DrawLine(x + 3, y, x, y + 6, color);
    LCD_DrawLine(x + 3, y, x + 6, y + 6, color);
    LCD_DrawCircle(x + 3, y + 7, 3, color);
    LCD_FillCircle(x + 3, y + 7, 1, theme_blue);
}

/* 绘制气压表图标 (注释以消除未引用警告)
static void Draw_GaugeIcon(uint16_t x, uint16_t y, uint16_t color) {
    LCD_DrawCircle(x + 6, y + 6, 6, color);
    LCD_DrawLine(x + 6, y + 6, x + 9, y + 3, theme_accent);
    LCD_DrawPoint(x + 6, y + 6, color);
}
*/

/* 绘制大脑 (AI) 节点图标 */
static void Draw_BrainIcon(uint16_t x, uint16_t y, uint16_t color) {
    LCD_DrawLine(x + 3, y + 1, x + 1, y + 6, theme_border);
    LCD_DrawLine(x + 3, y + 1, x + 7, y + 4, theme_border);
    LCD_DrawLine(x + 1, y + 6, x + 5, y + 10, theme_border);
    LCD_DrawLine(x + 7, y + 4, x + 5, y + 10, theme_border);
    LCD_DrawLine(x + 1, y + 6, x + 7, y + 4, theme_border);
    LCD_FillCircle(x + 3, y + 1, 1, color);
    LCD_FillCircle(x + 1, y + 6, 1, color);
    LCD_FillCircle(x + 7, y + 4, 1, color);
    LCD_FillCircle(x + 5, y + 10, 1, color);
}

/* 绘制点虚线（实现科技示波网格） */
static void Draw_DottedLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    if (x1 == x2) {
        for (uint16_t y = y1; y <= y2; y += 4) {
            LCD_DrawPoint(x1, y, color);
            if (y + 1 <= y2) LCD_DrawPoint(x1, y + 1, color);
        }
    } else if (y1 == y2) {
        for (uint16_t x = x1; x <= x2; x += 4) {
            LCD_DrawPoint(x, y1, color);
            if (x + 1 <= x2) LCD_DrawPoint(x + 1, y1, color);
        }
    }
}

/* 绘制分段式进度条 */
static void Draw_SegmentedBar(uint16_t x, uint16_t y, uint16_t active_segs, uint16_t total_segs, uint16_t active_color, uint16_t inactive_color) {
    for (uint16_t i = 0; i < total_segs; i++) {
        uint16_t sx = x + i * 15;
        uint16_t color = (i < active_segs) ? active_color : inactive_color;
        LCD_FillRect(sx, y, 12, 6, color);
    }
}

/* ---- 获取当前状态主色调 ---- */
static uint16_t TopBarBg(void) {
    if (!aht20_healthy) return theme_red;
    if (current_ai_state == AI_STATE_ANOMALY) return theme_red;
    if (current_ai_state == AI_STATE_LEARNING) return theme_blue;
    return theme_card_bg;
}

/* ---- 绘制顶部状态栏（科技徽章风格） ---- */
static void Draw_TopBar(void) {
    uint16_t bg = TopBarBg();
    
    // 清空背景
    LCD_FillRect(0, 0, LCD_WIDTH, TOP_BAR_H, theme_bg);
    
    // 霓虹底部双色线
    uint16_t line_color = (bg == theme_red) ? theme_red : ((bg == theme_blue) ? theme_blue : theme_accent);
    LCD_DrawLine(0, TOP_BAR_H - 1, LCD_WIDTH - 1, TOP_BAR_H - 1, line_color);
    
    char buf[40];
    uint16_t text_color;
    if (!aht20_healthy) {
        strcpy(buf, " SENSOR ERROR ");
        text_color = theme_red;
    } else if (current_ai_state == AI_STATE_ANOMALY) {
        strcpy(buf, " ANOMALY ALARM ");
        text_color = theme_red;
    } else if (current_ai_state == AI_STATE_LEARNING) {
        strcpy(buf, " AI LEARNING ");
        text_color = theme_blue;
    } else {
        strcpy(buf, " SYSTEM SAFE ");
        text_color = theme_green;
    }

    uint16_t text_len = (uint16_t)strlen(buf);
    uint16_t pill_w = text_len * 8 + 12;
    uint16_t pill_h = 16;
    uint16_t pill_x = (LCD_WIDTH - pill_w) / 2;
    uint16_t pill_y = (TOP_BAR_H - pill_h) / 2;
    
    LCD_DrawRect(pill_x, pill_y, pill_w, pill_h, text_color);
    Draw_CornerBrackets(pill_x, pill_y, pill_w, pill_h, 3, text_color);
    LCD_ShowString(pill_x + 6, pill_y + 1, buf, text_color, theme_bg);

    // 绘制主题切换空按钮 (X: 420, Y: 4, W: 24, H: 16)
    LCD_DrawRect(420, 4, 24, 16, theme_border);
    if (current_theme == 0) {
        // 暗色模式：填充主题高亮色
        LCD_FillRect(426, 8, 12, 8, theme_accent);
    } else {
        // 亮色模式：保留空心框
        LCD_DrawRect(426, 8, 12, 8, theme_accent);
    }
}

/* ---- 绘制副标题（带有科技几何装饰线） ---- */
static void Draw_Header(const char *title) {
    LCD_DrawLine(0, SEP_Y1, LCD_WIDTH - 1, SEP_Y1, theme_border);
    LCD_FillRect(0, SEP_Y1 + 1, LCD_WIDTH, SEP_Y2 - SEP_Y1 - 1, theme_bg);
    
    // Left vertical accent bar for premium look
    LCD_FillRect(8, TITLE_Y, 3, 16, theme_accent);
    
    LCD_ShowString(16, TITLE_Y, title, theme_accent, theme_bg);
    
    // 右侧科技感短斜线装饰
    LCD_DrawLine(LCD_WIDTH - 60, TITLE_Y + 4, LCD_WIDTH - 24, TITLE_Y + 4, theme_border);
    LCD_DrawLine(LCD_WIDTH - 24, TITLE_Y + 4, LCD_WIDTH - 20, TITLE_Y, theme_border);
    
    LCD_DrawLine(0, SEP_Y2, LCD_WIDTH - 1, SEP_Y2, theme_border);
}

/* ---- 绘制底部翻页指示点（呼吸棱形） ---- */
static void Draw_PageDots(void) {
    uint16_t dots[5] = {200, 220, 240, 260, 280};
    for (uint8_t i = 0; i < 5; i++) {
        if (i == current_page) {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 3, theme_accent);
            LCD_DrawCircle(dots[i], PAGE_DOT_Y, 5, theme_accent);
        } else {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 2, theme_bg);
            LCD_DrawCircle(dots[i], PAGE_DOT_Y, 3, theme_border);
        }
    }
}

/* ---- 底部系统信息 ---- */
static void Draw_BottomInfo(void) {
    char buf[24];
    sprintf(buf, "SYS-UP: %5ds", system_uptime_s);
    LCD_FillRect(0, BOTTOM_Y, 150, 16, theme_bg);
    LCD_ShowString(16, BOTTOM_Y, buf, theme_text_muted, theme_bg);

    sprintf(buf, "G-05 HUD");
    uint16_t x = LCD_WIDTH - (uint16_t)strlen(buf) * 8 - 16;
    LCD_FillRect(x, BOTTOM_Y, 80, 16, theme_bg);
    LCD_ShowString(x, BOTTOM_Y, buf, theme_text_muted, theme_bg);
}

/* ---- Page 1: 偏差网格折线图（示波器风格） ---- */
static void Draw_DevChart(void) {
    uint16_t cx = 216, cy = 56, cw = 248, ch = 220;
    uint16_t zero_y = cy + ch / 2;
    
    // 清空图表内部区域，防止历史折线残留堆叠
    LCD_FillRect(cx + 1, cy + 8, cw - 2, ch - 10, theme_card_bg);
    
    // 重新绘制图表标题
    LCD_ShowString(cx + 12, cy + 6, "Deviation Trend (+/-5 C)", theme_accent, theme_card_bg);
    
    // 绘制示波器点虚线网格
    Draw_DottedLine(cx + 6, zero_y - 45, cx + cw - 7, zero_y - 45, theme_border);  /* +3.0C */
    Draw_DottedLine(cx + 6, zero_y + 45, cx + cw - 7, zero_y + 45, theme_border);  /* -3.0C */
    
    for (uint16_t gx = cx + 30; gx < cx + cw - 10; gx += 38) {
        Draw_DottedLine(gx, cy + 10, gx, cy + ch - 10, theme_border);
    }
    
    Draw_DottedLine(cx + 6, zero_y, cx + cw - 7, zero_y, theme_text_muted);       /* 零偏差线 */
    LCD_DrawLine(cx + 6, cy + 10, cx + 6, cy + ch - 10, theme_border);         /* 左轴 */
    LCD_DrawLine(cx + cw - 6, cy + 10, cx + cw - 6, cy + ch - 10, theme_border); /* 右轴 */

    /* 数据折线绘制 */
    float scale = 15.0f;
    for (uint8_t i = 0; i < 23; i++) {
        uint16_t x1 = cx + 12 + i * 10;
        uint16_t x2 = cx + 12 + (i + 1) * 10;
        float y1_val = (float)zero_y - dev_history[i]   * scale;
        float y2_val = (float)zero_y - dev_history[i+1] * scale;

        if (y1_val < cy + 10) y1_val = cy + 10;
        if (y1_val > cy + ch - 10) y1_val = cy + ch - 10;
        if (y2_val < cy + 10) y2_val = cy + 10;
        if (y2_val > cy + ch - 10) y2_val = cy + ch - 10;

        LCD_DrawLine(x1, (uint16_t)y1_val, x2, (uint16_t)y2_val, theme_accent);
        LCD_FillCircle(x2, (uint16_t)y2_val, 1, theme_accent); // 绘制数据接点
    }
}

/* ---- Page 4: 距离网格历史曲线 ---- */
static void Draw_DistanceChart(void) {
    uint16_t cx = 216, cy = 56, cw = 248, ch = 220;
    
    // 清空图表内部区域
    LCD_FillRect(cx + 1, cy + 8, cw - 2, ch - 10, theme_card_bg);
    
    // 重新绘制图表标题
    LCD_ShowString(cx + 12, cy + 6, "Distance Trend (0-150cm)", theme_accent, theme_card_bg);
    
    // 绘制示波器点虚线网格 (例如 50cm 和 100cm 刻度)
    // 150cm 高度 -> scale: (ch - 20) / 150.0f = 200 / 150.0f = 1.333f
    float scale = 1.333f;
    uint16_t y_50 = cy + ch - 10 - (uint16_t)(50.0f * scale);
    uint16_t y_100 = cy + ch - 10 - (uint16_t)(100.0f * scale);
    
    Draw_DottedLine(cx + 6, y_50, cx + cw - 7, y_50, theme_border);  /* 50cm 线 */
    Draw_DottedLine(cx + 6, y_100, cx + cw - 7, y_100, theme_border); /* 100cm 线 */
    
    for (uint16_t gx = cx + 30; gx < cx + cw - 10; gx += 38) {
        Draw_DottedLine(gx, cy + 10, gx, cy + ch - 10, theme_border);
    }
    
    LCD_DrawLine(cx + 6, cy + 10, cx + 6, cy + ch - 10, theme_border);         /* 左轴 */
    LCD_DrawLine(cx + cw - 6, cy + 10, cx + cw - 6, cy + ch - 10, theme_border); /* 右轴 */

    /* 历史数据折线绘制 */
    for (uint8_t i = 0; i < 23; i++) {
        uint16_t x1 = cx + 12 + i * 10;
        uint16_t x2 = cx + 12 + (i + 1) * 10;
        float d1 = distance_history[i];
        float d2 = distance_history[i+1];
        if (d1 > 150.0f) d1 = 150.0f;
        if (d2 > 150.0f) d2 = 150.0f;
        float y1_val = (float)(cy + ch - 10) - d1 * scale;
        float y2_val = (float)(cy + ch - 10) - d2 * scale;

        if (y1_val < cy + 10) y1_val = cy + 10;
        if (y1_val > cy + ch - 10) y1_val = cy + ch - 10;
        if (y2_val < cy + 10) y2_val = cy + 10;
        if (y2_val > cy + ch - 10) y2_val = cy + ch - 10;

        LCD_DrawLine(x1, (uint16_t)y1_val, x2, (uint16_t)y2_val, theme_accent);
        LCD_FillCircle(x2, (uint16_t)y2_val, 1, theme_accent); // 绘制数据接点
    }
}

/* ============================================================
 * Display_Refresh — 主 UI 绘制函数
 * ============================================================ */
void Display_Refresh(uint8_t force_refresh) {
    char buf[40];

    /* 切页 → 全屏清屏 */
    if (current_page != last_page || force_refresh) {
        LCD_Clear(theme_bg);
        last_page = current_page;
        force_refresh = 1;
    }

    /* ---- 顶部状态栏 ---- */
    Draw_TopBar();

    /* ---- 标题 ---- */
    if (force_refresh) {
        if (current_page == 0)
            Draw_Header("[1] REAL-TIME MONITORING");
        else if (current_page == 1)
            Draw_Header("[2] AI BASELINE LEARNING");
        else if (current_page == 2)
            Draw_Header("[3] ANOMALY HISTORY LOGS");
        else if (current_page == 3)
            Draw_Header("[4] FLAPPY BIRD MINI-GAME");
        else if (current_page == 4)
            Draw_Header("[5] SECURITY GUARD MODE");
    }

    /* ================================================================
     * PAGE 0: 实时监测 (HUD 两栏卡片网格布局)
     * ================================================================ */
    if (current_page == 0) {
        
        // ------------------ 左侧栏 ------------------
        
        // Card 1: Temperature Raw (Y: 56..156, height 100)
        if (force_refresh) {
            LCD_FillRect(16, 56, 216, 100, theme_card_bg);
            LCD_DrawRect(16, 56, 216, 100, theme_border);
            Draw_CornerBrackets(16, 56, 216, 100, 6, theme_accent);
            Draw_TempIcon(28, 66, theme_accent);
            LCD_ShowString(42, 66, "TEMP RAW:", theme_accent, theme_card_bg);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", test_aht20_temp);
            LCD_ShowString(28, 90, buf, theme_text, theme_card_bg);
            
            float t = test_aht20_temp;
            if (t < 10.0f) t = 10.0f;
            if (t > 40.0f) t = 40.0f;
            uint16_t active_segs = (uint16_t)((t - 10.0f) * 12.0f / 30.0f);
            uint16_t bar_color = (t > 30.0f || t < 15.0f) ? theme_red : theme_accent;
            Draw_SegmentedBar(36, 122, active_segs, 12, bar_color, theme_bg);
        } else {
            LCD_ShowString(28, 90, "[ERROR]", theme_red, theme_card_bg);
            Draw_SegmentedBar(36, 122, 0, 12, theme_red, theme_bg);
        }

        // Card 2: Humidity (Y: 176..276, height 100)
        if (force_refresh) {
            LCD_FillRect(16, 176, 216, 100, theme_card_bg);
            LCD_DrawRect(16, 176, 216, 100, theme_border);
            Draw_CornerBrackets(16, 176, 216, 100, 6, theme_accent);
            Draw_DropIcon(28, 186, theme_accent);
            LCD_ShowString(40, 186, "HUMIDITY:", theme_accent, theme_card_bg);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f %%", test_aht20_humi);
            LCD_ShowString(28, 210, buf, theme_text, theme_card_bg);
            
            float h = test_aht20_humi;
            if (h < 0.0f) h = 0.0f;
            if (h > 100.0f) h = 100.0f;
            uint16_t active_segs = (uint16_t)(h * 12.0f / 100.0f);
            Draw_SegmentedBar(36, 242, active_segs, 12, theme_accent, theme_bg);
        } else {
            LCD_ShowString(28, 210, "[ERROR]", theme_red, theme_card_bg);
            Draw_SegmentedBar(36, 242, 0, 12, theme_red, theme_bg);
        }

        // ------------------ 右侧栏 ------------------
        
        // Card 3: AI Filtered Temp & State (Y: 56..156, height 100)
        if (force_refresh) {
            LCD_FillRect(248, 56, 216, 100, theme_card_bg);
            LCD_DrawRect(248, 56, 216, 100, theme_border);
            Draw_CornerBrackets(248, 56, 216, 100, 6, theme_accent);
            Draw_BrainIcon(260, 66, theme_accent);
            LCD_ShowString(274, 66, "AI FILTERED:", theme_accent, theme_card_bg);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", test_filtered_temp);
            LCD_ShowString(260, 88, buf, theme_text, theme_card_bg);
            
            if (current_ai_state == AI_STATE_LEARNING) {
                uint8_t pct = (my_detector.learning_samples * 100) / my_detector.max_learning_samples;
                sprintf(buf, "STATE: LEARN (%d%%)", pct);
                LCD_ShowString(260, 114, buf, theme_blue, theme_card_bg);
            } else if (current_ai_state == AI_STATE_NORMAL) {
                LCD_ShowString(260, 114, "STATE: SAFE", theme_green, theme_card_bg);
            } else {
                LCD_ShowString(260, 114, "STATE: ANOMALY!!", theme_red, theme_card_bg);
            }
            LCD_ShowString(260, 134, "(EMA alpha=0.2)", theme_text_muted, theme_card_bg);
        } else {
            LCD_ShowString(260, 88, "[ERROR]", theme_red, theme_card_bg);
            LCD_ShowString(260, 114, "STATE: SENSOR ERR", theme_red, theme_card_bg);
        }

        // Card 4: Hardware Status (Y: 176..276, height 100)
        if (force_refresh) {
            LCD_FillRect(248, 176, 216, 100, theme_card_bg);
            LCD_DrawRect(248, 176, 216, 100, theme_border);
            Draw_CornerBrackets(248, 176, 216, 100, 6, theme_accent);
            LCD_ShowString(260, 184, "HARDWARE STATUS:", theme_accent, theme_card_bg);
            LCD_ShowString(260, 202, "AHT20 :", theme_text_muted, theme_card_bg);
            LCD_ShowString(260, 220, "BH175 :", theme_text_muted, theme_card_bg);
            LCD_ShowString(260, 238, "SD LOG:", theme_text_muted, theme_card_bg);
        }
        
        /* AHT20 online status */
        LCD_ShowString(324, 202, aht20_healthy ? "ONLINE " : "OFFLINE", aht20_healthy ? theme_green : theme_red, theme_card_bg);
        LCD_FillCircle(316, 209, 3, aht20_healthy ? theme_green : theme_red);
        
        /* BH1750 online status */
        LCD_ShowString(324, 220, bh1750_healthy ? "ONLINE " : "OFFLINE", bh1750_healthy ? theme_green : theme_red, theme_card_bg);
        LCD_FillCircle(316, 227, 3, bh1750_healthy ? theme_green : theme_red);
        
        /* SD Card online status */
        LCD_ShowString(324, 238, sd_healthy ? "ONLINE " : "OFFLINE", sd_healthy ? theme_green : theme_red, theme_card_bg);
        LCD_FillCircle(316, 245, 3, sd_healthy ? theme_green : theme_red);
        
        /* Light Intensity lux raw reading */
        if (bh1750_healthy) {
            sprintf(buf, "LIGHT : %5.1f lx", test_bh1750_lux);
            LCD_ShowString(260, 256, buf, theme_yellow, theme_card_bg);
        } else {
            LCD_ShowString(260, 256, "LIGHT : [ERROR]  ", theme_red, theme_card_bg);
        }
    }

    /* ================================================================
     * PAGE 1: AI自适应基准学习 (左栏 HUD 卡片 + 右栏趋势图)
     * ================================================================ */
    else if (current_page == 1) {
        
        // ------------------ 左侧诊断卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(16, 56, 184, 220, theme_card_bg);
            LCD_DrawRect(16, 56, 184, 220, theme_border);
            Draw_CornerBrackets(16, 56, 184, 220, 6, theme_accent);
            
            Draw_TempIcon(24, 62, theme_green);
            LCD_ShowString(38, 62, "Baseline Temp:", theme_accent, theme_card_bg);
            
            Draw_BrainIcon(24, 96, theme_yellow);
            LCD_ShowString(38, 96, "Filtered Temp:", theme_accent, theme_card_bg);
            
            LCD_ShowString(24, 130, "Deviation:", theme_accent, theme_card_bg);
            LCD_ShowString(24, 204, "AI Progress:", theme_accent, theme_card_bg);
        }
        
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", my_detector.baseline_temp);
            LCD_ShowString(24, 78, buf, theme_green, theme_card_bg);
            
            sprintf(buf, "%5.2f C", test_filtered_temp);
            LCD_ShowString(24, 112, buf, theme_yellow, theme_card_bg);
            
            float dev = test_filtered_temp - my_detector.baseline_temp;
            sprintf(buf, "%+5.2f C", dev);
            uint16_t dc = theme_green;
            if (dev > 3.0f || dev < -3.0f) dc = theme_red;
            else if (dev > 1.5f || dev < -1.5f) dc = theme_yellow;
            LCD_ShowString(24, 146, buf, dc, theme_card_bg);
            
            // 偏差指示小彩条 (X:24..176, width 152)
            LCD_FillRect(24, 168, 152, 6, theme_bg);
            LCD_DrawRect(24, 168, 152, 6, theme_border);
            LCD_DrawLine(100, 166, 100, 171, theme_border); // 中心零刻度
            
            float dev_lim = dev;
            if (dev_lim > 5.0f) dev_lim = 5.0f;
            if (dev_lim < -5.0f) dev_lim = -5.0f;
            int16_t w = (int16_t)(dev_lim * 15.0f); // 5C 映射至 75px
            if (w > 0) {
                LCD_FillRect(100, 169, w, 4, dc);
            } else if (w < 0) {
                LCD_FillRect(100 + w, 169, -w, 4, dc);
            }
        } else {
            LCD_ShowString(24, 78, "[ERROR]", theme_red, theme_card_bg);
            LCD_ShowString(24, 112, "[ERROR]", theme_red, theme_card_bg);
            LCD_ShowString(24, 146, "[ERROR]", theme_red, theme_card_bg);
        }
        
        // 学习样本进度条
        sprintf(buf, "%3d / %d", my_detector.learning_samples, my_detector.max_learning_samples);
        LCD_ShowString(24, 222, buf, theme_text, theme_card_bg);
        
        {
            uint32_t cur = my_detector.learning_samples;
            uint32_t max = my_detector.max_learning_samples;
            if (cur > max) cur = max;
            uint16_t active_segs = (uint16_t)(cur * 10 / max);
            uint16_t pc = my_detector.is_learning_done ? theme_green : theme_blue;
            Draw_SegmentedBar(24, 244, active_segs, 10, pc, theme_bg);
        }

        // ------------------ 右侧趋势图卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(216, 56, 248, 220, theme_card_bg);
            LCD_DrawRect(216, 56, 248, 220, theme_border);
            Draw_CornerBrackets(216, 56, 248, 220, 6, theme_border);
            LCD_ShowString(228, 62, "Deviation Trend (+/-5 C)", theme_accent, theme_card_bg);
        }
        Draw_DevChart();
    }

    /* ================================================================
     * PAGE 2: 历史日志 (科技感时空节点布局)
     * ================================================================ */
    else if (current_page == 2) {
        if (my_log_buffer.count == 0) {
            if (force_refresh) {
                LCD_ShowString(120, 150, "NO ANOMALY LOGS RECORDED", theme_text_muted, theme_bg);
            }
        } else {
            /* 1. 先绘制点虚线时间轴垂直线 */
            if (force_refresh) {
                Draw_DottedLine(24, 56, 24, 276, theme_border);
            }
            
            /* 2. 遍历环形缓冲区绘制历史异常记录 */
            uint8_t show_count = my_log_buffer.count > 4 ? 4 : my_log_buffer.count;
            uint8_t start_idx = (my_log_buffer.head + MAX_ANOMALY_LOGS - my_log_buffer.count) % MAX_ANOMALY_LOGS;
            
            for (uint8_t i = 0; i < show_count; i++) {
                uint8_t idx = (start_idx + (my_log_buffer.count - show_count) + i) % MAX_ANOMALY_LOGS;
                AnomalyEvent *ev = &my_log_buffer.logs[idx];
                uint16_t row_y = 56 + i * 54; /* 4个卡片，高度48，间距6 */

                if (force_refresh) {
                    /* 双圈闪烁时间轴节点 */
                    LCD_DrawLine(24, row_y + 24, 42, row_y + 24, theme_border);
                    LCD_FillCircle(24, row_y + 24, 2, theme_red);
                    LCD_DrawCircle(24, row_y + 24, 5, theme_red);

                    /* 右侧日志卡片背景与细灰色边框，配合红角标 */
                    LCD_FillRect(42, row_y, 422, 48, theme_card_bg);
                    LCD_DrawRect(42, row_y, 422, 48, theme_border);
                    Draw_CornerBrackets(42, row_y, 422, 48, 4, theme_red);

                    /* 卡片内部信息 */
                    sprintf(buf, "#%d [TIME: %d s]", i + 1, ev->timestamp_s);
                    LCD_ShowString(52, row_y + 6, buf, theme_yellow, theme_card_bg);

                    sprintf(buf, "Base: %5.2f C -> Curr: %5.2f C", ev->baseline_temp, ev->current_temp);
                    LCD_ShowString(52, row_y + 26, buf, theme_red, theme_card_bg);


                }
            }
        }
    }
    
    /* ================================================================
     * PAGE 3: Flappy Bird 像素飞鸟小游戏
     * ================================================================ */
    else if (current_page == 3) {
        Game_Draw(force_refresh);
    }
    
    /* ================================================================
     * PAGE 4: 防盗监控 (左栏 HUD 卡片 + 右栏趋势图)
     * ================================================================ */
    else if (current_page == 4) {
        // ------------------ 左侧诊断卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(16, 56, 184, 220, theme_card_bg);
            LCD_DrawRect(16, 56, 184, 220, theme_border);
            Draw_CornerBrackets(16, 56, 184, 220, 6, theme_accent);
            
            LCD_ShowString(24, 62, "Security Mode:", theme_accent, theme_card_bg);
            LCD_ShowString(24, 130, "Current Dist:", theme_accent, theme_card_bg);
            LCD_ShowString(24, 204, "Alarm Status:", theme_accent, theme_card_bg);
        }
        
        // Security mode text
        if (security_alert_mode) {
            LCD_ShowString(24, 86, "ARMED    ", theme_red, theme_card_bg);
        } else {
            LCD_ShowString(24, 86, "DISARMED ", theme_text_muted, theme_card_bg);
        }
        
        // Current distance text
        if (ultrasonic_healthy) {
            sprintf(buf, "%5.1f cm", test_ultrasonic_dist);
            LCD_ShowString(24, 154, buf, theme_text, theme_card_bg);
            
            float dist = test_ultrasonic_dist;
            if (dist > 100.0f) dist = 100.0f;
            if (dist < 0.0f) dist = 0.0f;
            uint16_t active_segs = (uint16_t)(dist * 10.0f / 100.0f);
            uint16_t bar_color = (dist < 15.0f) ? theme_red : ((dist < 50.0f) ? theme_yellow : theme_green);
            Draw_SegmentedBar(24, 178, active_segs, 10, bar_color, theme_bg);
        } else {
            LCD_ShowString(24, 154, "[ERROR]  ", theme_red, theme_card_bg);
            Draw_SegmentedBar(24, 178, 0, 10, theme_red, theme_bg);
        }
        
        // Alarm status text
        if (!security_alert_mode) {
            LCD_ShowString(24, 228, "INACTIVE", theme_text_muted, theme_card_bg);
        } else {
            if (test_ultrasonic_dist < 15.0f) {
                LCD_ShowString(24, 228, "ALARM!!!", theme_red, theme_card_bg);
            } else if (test_ultrasonic_dist < 50.0f) {
                LCD_ShowString(24, 228, "WARNING ", theme_yellow, theme_card_bg);
            } else {
                LCD_ShowString(24, 228, "SAFE    ", theme_green, theme_card_bg);
            }
        }
        
        // ------------------ 右侧趋势图卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(216, 56, 248, 220, theme_card_bg);
            LCD_DrawRect(216, 56, 248, 220, theme_border);
            Draw_CornerBrackets(216, 56, 248, 220, 6, theme_border);
        }
        Draw_DistanceChart();
    }

    /* ---- 底部信息 & 翻页点 ---- */
    Draw_BottomInfo();
    Draw_PageDots();
}

/* ---- 心跳 Beacon ---- */
static void Draw_Beacon(void) {
    static uint8_t  idx  = 0;
    static uint32_t tick = 0;
    const char sym[] = {'|', '/', '-', '\\'};

    tick++;
    if (tick >= 20) {
        tick = 0;
        char s[2] = {sym[idx], '\0'};
        uint16_t bg = TopBarBg();
        uint16_t fg = (bg == theme_red) ? theme_red : ((bg == theme_blue) ? theme_blue : theme_green);
        LCD_ShowString(LCD_WIDTH - 16, 4, s, fg, theme_bg);
        idx = (idx + 1) & 3;
    }
}

/* ---- KEY3 Scan for security mode toggle ---- */
static uint8_t KEY3_Scan(void) {
    static uint8_t key3_up = 1;
    uint8_t key3_low = (GPIOB->IDR & GPIO_Pin_9) ? 0 : 1;
    if (key3_up && key3_low) {
        // Simple software delay debounce
        for (volatile uint32_t i = 0; i < 7200 * 20; i++);
        key3_low = (GPIOB->IDR & GPIO_Pin_9) ? 0 : 1;
        if (key3_low) {
            key3_up = 0;
            return 1;
        }
    } else if (!key3_low) {
        key3_up = 1;
    }
    return 0;
}

/* ============================================================
 * main
 * ============================================================ */
int main(void) {
    uint32_t loop_cnt       = 0;
    uint32_t flash_cnt      = 0;
    uint32_t uptime_cnt     = 0;
    uint8_t  ui_dirty       = 0;

    /* 硬件初始化 */
    LED_Init();
    KEY_Init();
    LCD_Init();
    Theme_Init();
    LCD_Clear(theme_bg);

    /* 初始化 BH1750 (软件 I2C) */
    bh1750_healthy = (BH1750_Init() == 0);
    
    /* 重新配置并启用 SD 卡 (MISO已移至PA11) */
    sd_healthy = (SD_Init() == SD_RESPONSE_NO_ERROR);
    
    /* 初始化 TM1637 数码管及 HC-SR04 超声波测距 */
    TM1637_Init();
    SR04_Init();
    
    /* 配置面包板 KEY3 按键 (PB9 上拉输入) 和蜂鸣器 (PB15 推挽输出) */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR;
    
    GPIOB->CRH &= ~0x000000F0;
    GPIOB->CRH |=  0x00000080;
    GPIOB->ODR |= GPIO_Pin_9;  // 开启上拉
    
    GPIOB->CRH &= ~0xF0000000;
    GPIOB->CRH |=  0x30000000;
    GPIOB->BRR = GPIO_Pin_15;  // 初始静音

    if (sd_healthy) {
        /* 自动扫描 SD 卡空闲扇区以恢复数据记录 */
        uint8_t temp_sector[512];
        log_sector_cursor = 1000;
        while (log_sector_cursor < 20000) {
            if (SD_ReadBlock(temp_sector, log_sector_cursor, 512) == SD_RESPONSE_NO_ERROR) {
                if (temp_sector[0] == 'U' && temp_sector[1] == 'P' && temp_sector[2] == ':') {
                    log_sector_cursor++;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }

    /* 传感器初始化 */
    test_aht20_init_status  = AHT20_Init();
    aht20_healthy           = (test_aht20_init_status  == 0);

    /* AI & 日志初始化 (适配 MAX_ANOMALY_LOGS 为 7) */
    AI_Init(&my_detector, 0.2f, 100);
    Log_Init(&my_log_buffer);

    LED_Off(0);
    LED_Off(1);

    Display_Refresh(1);

    /* ---- 主循环 ---- */
    while (1) {
        /* 秒计数器 */
        uptime_cnt++;
        if (uptime_cnt >= 100) { 
            uptime_cnt = 0; 
            system_uptime_s++; 
            
            /* 每秒数据向 SD 卡缓存写入一次 */
            if (sd_healthy) {
                char log_entry[65];
                int len = sprintf(log_entry, "UP:%5ds T:%5.2fC H:%5.2f%% L:%5.1flx A:%d M:%d",
                                  system_uptime_s,
                                  aht20_healthy ? test_aht20_temp : 0.0f,
                                  aht20_healthy ? test_aht20_humi : 0.0f,
                                  bh1750_healthy ? test_bh1750_lux : 0.0f,
                                  current_ai_state,
                                  system_mode);
                
                /* 空格填充至 62 字节，最后加 \r\n 并以 \0 结尾 */
                for (int i = len; i < 62; i++) {
                    log_entry[i] = ' ';
                }
                log_entry[62] = '\r';
                log_entry[63] = '\n';
                log_entry[64] = '\0';
                
                memcpy(&log_buffer[log_buffer_index * 64], log_entry, 64);
                log_buffer_index++;
                
                /* 累积满 8 条数据 (512 字节 = 1 扇区) 时进行写入 */
                if (log_buffer_index >= 8) {
                    log_buffer_index = 0;
                    if (SD_WriteBlock(log_buffer, log_sector_cursor, 512) == SD_RESPONSE_NO_ERROR) {
                        log_sector_cursor++;
                        if (log_sector_cursor >= 25000) {
                            log_sector_cursor = 1000; /* 循环写保护，防溢出 */
                        }
                    } else {
                        sd_healthy = 0; /* 标记为异常，自动在 1.5s 周期内重连 */
                    }
                }
            }
        }

        /* 按键扫描与缓存机制 (以防止 30ms 游戏循环漏按) */
        uint8_t key = KEY_Scan(0);
        static uint8_t game_key = 0;
        if (key != KEY_NONE) {
            game_key = key;
        }

        /* 扫描 KEY3 切换布防/撤防状态 */
        if (KEY3_Scan()) {
            security_alert_mode = !security_alert_mode;
            ui_dirty = 1;
            flash_cnt = 10; // 强制下一个循环立即更新 LED & 蜂鸣器
        }

        if (key == KEY1_PRESS) {
            /* KEY1 按键: 循环切换页面 (0 -> 1 -> 2 -> 3 -> 4 -> 0) */
            current_page = (current_page + 1) % 5;
            if (current_page == 3) {
                Game_Init(); /* 切到游戏页面时初始化游戏数据 */
            }
            Display_Refresh(1);
        } else if (key == KEY2_PRESS) {
            if (current_page != 3) {
                /* KEY2 按键 (其他常规页面模式): 硬件自愈重置 + 切换主题 */
                current_theme = !current_theme;
                Theme_Apply();
                
                AI_Init(&my_detector, 0.2f, 100);
                current_ai_state = AI_STATE_LEARNING;
                system_mode      = 0;
                aht20_fail_cnt   = 0;
                test_aht20_init_status  = AHT20_Init();
                aht20_healthy           = (test_aht20_init_status  == 0);
                LED_Off(0);
                Display_Refresh(1);
            }
        }

        /* 1.5 秒传感器采样轮询 */
        loop_cnt++;
        if (loop_cnt >= 150) {
            loop_cnt = 0;

            /* AHT20 */
            if (!aht20_healthy) {
                if (AHT20_Init() == 0) { aht20_healthy = 1; test_aht20_init_status = 0; aht20_fail_cnt = 0; }
            }
            if (aht20_healthy) {
                float t, h;
                if (AHT20_ReadData(&t, &h) == 0) {
                    test_aht20_temp = t; test_aht20_humi = h; aht20_fail_cnt = 0;
                } else {
                    aht20_fail_cnt++;
                    if (aht20_fail_cnt >= 3) aht20_healthy = 0;
                }
            }

            /* BH1750 */
            if (!bh1750_healthy) {
                if (BH1750_Init() == 0) { bh1750_healthy = 1; }
            }
            if (bh1750_healthy) {
                float lux;
                if (BH1750_ReadData(&lux) == 0) {
                    test_bh1750_lux = lux;
                } else {
                    bh1750_healthy = 0;
                }
            }

            /* SD Card 自动重连 */
            if (!sd_healthy) {
                if (SD_Init() == SD_RESPONSE_NO_ERROR) {
                    sd_healthy = 1;
                }
            }

            /* 轮询超声波测距传感器并滚动历史记录 */
            {
                float new_dist = 0.0f;
                if (SR04_GetDistance(&new_dist) == 0) {
                    test_ultrasonic_dist = new_dist;
                    ultrasonic_healthy = 1;
                } else {
                    ultrasonic_healthy = 0;
                }
                
                for (uint8_t i = 0; i < 23; i++) {
                    distance_history[i] = distance_history[i + 1];
                }
                distance_history[23] = test_ultrasonic_dist;
            }

            /* TM1637 显示当前温度 */
            if (aht20_healthy) {
                TM1637_DisplayTemp(test_aht20_temp);
            } else {
                TM1637_DisplayClear();
            }

            /* AI 突变检测 */
            if (aht20_healthy) {
                float filt;
                uint8_t ns = AI_Process(&my_detector, test_aht20_temp, &filt);
                test_filtered_temp = filt;

                float dev = filt - my_detector.baseline_temp;
                for (uint8_t i = 0; i < 23; i++) dev_history[i] = dev_history[i+1];
                dev_history[23] = dev;

                if (current_ai_state != AI_STATE_ANOMALY && ns == AI_STATE_ANOMALY)
                    Log_Add(&my_log_buffer, system_uptime_s,
                            my_detector.baseline_temp, filt, 0.0f);
                current_ai_state = ns;
            }

            system_mode = (current_ai_state == AI_STATE_ANOMALY || !aht20_healthy);
            ui_dirty = 1;
        }

        /* UI 周期更新 */
        if (ui_dirty) { 
            ui_dirty = 0; 
            Display_Refresh(0); 
        }

        /* 游戏更新与刷新 (30ms 运行) */
        static uint8_t game_div = 0;
        if (current_page == 3) {
            game_div++;
            if (game_div >= 3) { /* 3 * 10ms = 30ms */
                game_div = 0;
                Game_Update(game_key);
                game_key = KEY_NONE;
                Game_Draw(0);
            }
        } else {
            game_key = KEY_NONE;
        }

        /* Beacon 心跳 */
        Draw_Beacon();

        /* LED与蜂鸣器综合状态机调度 (5Hz频率, 每100ms周期刷新) */
        LED_Off(0); // 核心板载 LED PC13 保持常灭
        
        flash_cnt++;
        if (flash_cnt >= 10) { // 10 * 10ms = 100ms (5Hz 周期)
            flash_cnt = 0;
            
            if (security_alert_mode) {
                /* ==================== ARMED 布防模式 ==================== */
                if (!ultrasonic_healthy) {
                    // 超声波异常：黄色 LED 闪烁，其余常灭，蜂鸣器关闭
                    LED_Off(1);
                    LED_Toggle(2);
                    LED_Off(3);
                    GPIOB->BRR = GPIO_Pin_15;
                } else {
                    float dist = test_ultrasonic_dist;
                    if (dist < 15.0f) {
                        // 报警级（<15cm）：红、黄、绿三色 LED 同步闪烁，蜂鸣器常鸣
                        LED_Toggle(1);
                        LED_Toggle(2);
                        LED_Toggle(3);
                        
                        // 同步所有 LED 的物理状态
                        uint8_t sync_state = (GPIOA->ODR & GPIO_Pin_1) ? 1 : 0;
                        LED_Write(1, sync_state);
                        LED_Write(2, sync_state);
                        LED_Write(3, sync_state);
                        
                        GPIOB->BSRR = GPIO_Pin_15;
                    } else if (dist < 50.0f) {
                        // 警告级（15~50cm）：黄色 LED 常亮，其余常灭，蜂鸣器关闭
                        LED_Off(1);
                        LED_On(2);
                        LED_Off(3);
                        GPIOB->BRR = GPIO_Pin_15;
                    } else {
                        // 安全级（>50cm）：绿色 LED 常亮，其余常灭，蜂鸣器关闭
                        LED_Off(1);
                        LED_Off(2);
                        LED_On(3);
                        GPIOB->BRR = GPIO_Pin_15;
                    }
                }
            } else {
                /* ==================== DISARMED 撤防模式 ==================== */
                GPIOB->BRR = GPIO_Pin_15; // 撤防时蜂鸣器始终关闭
                
                if (!aht20_healthy || !bh1750_healthy || !sd_healthy) {
                    // 硬件故障：红色 LED 常亮，其余常灭
                    LED_On(1);
                    LED_Off(2);
                    LED_Off(3);
                } else if (current_ai_state == AI_STATE_ANOMALY) {
                    // 温度异常（偏差超过正负5C）：红色 LED 闪烁，其余常灭
                    LED_Toggle(1);
                    LED_Off(2);
                    LED_Off(3);
                } else if (current_ai_state == AI_STATE_LEARNING) {
                    // AI学习：黄色 LED 常亮，其余常灭
                    LED_Off(1);
                    LED_On(2);
                    LED_Off(3);
                } else {
                    // 安全监控中：绿色 LED 常亮，其余常灭
                    LED_Off(1);
                    LED_Off(2);
                    LED_On(3);
                }
            }
        }

        Delay(72000);   /* ~10ms 循环节 */
    }
}

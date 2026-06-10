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
#include "usart1.h"
#include "esp8266.h"
#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* ============================================================
 * 【期末视频录制 - 功能切换开关】
 * 说明：每次只能将下方的【其中一个】宏设置为 1，其他的必须设为 0！
 * ============================================================ */
#define RUN_MODE_ALL       0   // 1: 运行原版所有功能
#define RUN_MODE_ENV_AI    1   // 1: 录制环境监测与AI功能 (默认测试)
#define RUN_MODE_GAME      0   // 1: 录制 Flappy Bird 游戏
#define RUN_MODE_SECURITY  0   // 1: 录制超声波防盗报警功能
#define RUN_MODE_IOT       0   // 1: 录制 ESP8266 网络天气功能

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
 * ESP8266 IoT 全局状态变量与 NTP/微信推送参数
 * ============================================================ */
volatile uint8_t wifi_connected = 0;
volatile uint8_t wifi_hw_online = 0;
char current_net_time[24] = "2026-06-05 20:00:00";
float outdoor_temp = 0.0f;
char outdoor_weather[16] = "N/A";
uint32_t last_time_sync_sec = 0;

volatile uint16_t net_year = 2026;
volatile uint8_t net_month = 6;
volatile uint8_t net_day = 5;
volatile uint8_t net_hour = 20;
volatile uint8_t net_minute = 0;
volatile uint8_t net_second = 0;

static uint32_t last_wechat_alert_sec = 0;
const char* SERVER_CHAN_KEY = "SCT360551Tr979wHzigggHm2byctWnwZXD";

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
    uint16_t dots[6] = {190, 210, 230, 250, 270, 290};
    for (uint8_t i = 0; i < 6; i++) {
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
        else if (current_page == 5)
            Draw_Header("[6] IOT WEATHER & CLOCK");
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

        // Card 4: Hardware Status (Y: 176..286, height 110)
        if (force_refresh) {
            LCD_FillRect(248, 176, 216, 110, theme_card_bg);
            LCD_DrawRect(248, 176, 216, 110, theme_border);
            Draw_CornerBrackets(248, 176, 216, 110, 6, theme_accent);
            LCD_ShowString(260, 184, "HARDWARE STATUS:", theme_accent, theme_card_bg);
            LCD_ShowString(260, 202, "AHT20 :", theme_text_muted, theme_card_bg);
            LCD_ShowString(260, 218, "BH175 :", theme_text_muted, theme_card_bg);
            LCD_ShowString(260, 234, "SD LOG:", theme_text_muted, theme_card_bg);
            LCD_ShowString(260, 250, "WIFI  :", theme_text_muted, theme_card_bg);
        }
        
        /* AHT20 online status */
        LCD_ShowString(324, 202, aht20_healthy ? "ONLINE " : "OFFLINE", aht20_healthy ? theme_green : theme_red, theme_card_bg);
        LCD_FillCircle(316, 209, 3, aht20_healthy ? theme_green : theme_red);
        
        /* BH1750 online status */
        LCD_ShowString(324, 218, bh1750_healthy ? "ONLINE " : "OFFLINE", bh1750_healthy ? theme_green : theme_red, theme_card_bg);
        LCD_FillCircle(316, 225, 3, bh1750_healthy ? theme_green : theme_red);
        
        /* SD Card online status */
        LCD_ShowString(324, 234, sd_healthy ? "ONLINE " : "OFFLINE", sd_healthy ? theme_green : theme_red, theme_card_bg);
        LCD_FillCircle(316, 241, 3, sd_healthy ? theme_green : theme_red);
        
        /* WiFi online status */
        if (!wifi_hw_online) {
            LCD_ShowString(324, 250, "NO MOD ", theme_red, theme_card_bg);
            LCD_FillCircle(316, 257, 3, theme_red);
        } else if (wifi_connected) {
            LCD_ShowString(324, 250, "ONLINE ", theme_green, theme_card_bg);
            LCD_FillCircle(316, 257, 3, theme_green);
        } else {
            LCD_ShowString(324, 250, "DISC   ", theme_yellow, theme_card_bg);
            LCD_FillCircle(316, 257, 3, theme_yellow);
        }
        
        /* Light Intensity lux raw reading */
        if (bh1750_healthy) {
            sprintf(buf, "LIGHT : %5.1f lx", test_bh1750_lux);
            LCD_ShowString(260, 268, buf, theme_yellow, theme_card_bg);
        } else {
            LCD_ShowString(260, 268, "LIGHT : [ERROR]  ", theme_red, theme_card_bg);
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
    else if (current_page == 5) {
        if (force_refresh) {
            LCD_FillRect(16, 56, 448, 220, theme_card_bg);
            LCD_DrawRect(16, 56, 448, 220, theme_border);
            Draw_CornerBrackets(16, 56, 448, 220, 8, theme_accent);
            
            LCD_ShowString(32, 68, "NETWORK CLOCK & IoT WEATHER", theme_accent, theme_card_bg);
            LCD_DrawLine(32, 86, 448 - 32, 86, theme_border);
            
            LCD_ShowString(32, 100, "Indoor (AHT20):", theme_text, theme_card_bg);
            LCD_ShowString(240, 100, "Outdoor (Seniverse):", theme_text, theme_card_bg);
        }
        
        // Display Current Sync Time
        sprintf(buf, "TIME: %s", wifi_connected ? current_net_time : "Connecting...");
        LCD_ShowString(32, 190, buf, wifi_connected ? theme_green : theme_yellow, theme_card_bg);
        
        // Display Indoor Temp and Humidity
        if (aht20_healthy) {
            sprintf(buf, "Temp: %5.2f C", test_aht20_temp);
            LCD_ShowString(32, 124, buf, theme_text, theme_card_bg);
            sprintf(buf, "Humi: %5.2f %%", test_aht20_humi);
            LCD_ShowString(32, 148, buf, theme_text, theme_card_bg);
        } else {
            LCD_ShowString(32, 124, "Temp: [ERROR]", theme_red, theme_card_bg);
            LCD_ShowString(32, 148, "Humi: [ERROR]", theme_red, theme_card_bg);
        }
        
        // Display Outdoor Weather and Temp
        if (wifi_connected) {
            sprintf(buf, "Temp: %5.2f C", outdoor_temp);
            LCD_ShowString(240, 124, buf, theme_text, theme_card_bg);
            sprintf(buf, "Weat: %s", outdoor_weather);
            LCD_ShowString(240, 148, buf, theme_yellow, theme_card_bg);
            LCD_ShowString(240, 172, "Loc : Guangzhou", theme_text_muted, theme_card_bg);
        } else {
            LCD_ShowString(240, 124, "Temp: N/A", theme_text_muted, theme_card_bg);
            LCD_ShowString(240, 148, "Weat: N/A", theme_text_muted, theme_card_bg);
            LCD_ShowString(240, 172, "Loc : N/A", theme_text_muted, theme_card_bg);
        }
        
        // Display WiFi status
        if (wifi_connected) {
            sprintf(buf, "WiFi: Connected (%s)           ", "xiaobo");
            LCD_ShowString(32, 220, buf, theme_green, theme_card_bg);
        } else if (wifi_hw_online) {
            const char *err_msg = "UNKNOWN";
            if (wifi_conn_error == 1) err_msg = "TIMEOUT";
            else if (wifi_conn_error == 2) err_msg = "WRONG PWD";
            else if (wifi_conn_error == 3) err_msg = "NO AP FOUND";
            else if (wifi_conn_error == 4) err_msg = "CONN FAIL";
            else if (wifi_conn_error == 5) err_msg = "DHCP FAIL";
            else if (wifi_conn_error == 9) err_msg = "RESP TIMEOUT";
            else if (wifi_conn_error == 10) err_msg = "MODE ERR";
            else if (wifi_conn_error == 20) err_msg = "MUX ERR";
            
            sprintf(buf, "WiFi: Disconnected (%s)       ", err_msg);
            LCD_ShowString(32, 220, buf, theme_yellow, theme_card_bg);
        } else {
            LCD_ShowString(32, 220, "WiFi: No Module (HW ERR)     ", theme_red, theme_card_bg);
        }
        
        // Display raw AT debug log message
        snprintf(buf, sizeof(buf), "DBG : %-30s", wifi_debug_msg);
        LCD_ShowString(32, 242, buf, theme_text_muted, theme_card_bg);
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

/* ---- KEY3 Scan for security mode toggle ----
 * Fix: Replaced blocking busy-wait debounce (~20ms) with a non-blocking
 * counter-based state machine. The debounce now counts consecutive stable
 * samples across main loop ticks (~10ms each), so it never blocks the loop.
 * ---- */
static uint8_t KEY3_Scan(void) {
    static uint8_t key3_up      = 1;  /* 1 = released, 0 = pressed */
    static uint8_t debounce_cnt = 0;  /* stable-sample counter       */
    uint8_t key3_low = (GPIOB->IDR & GPIO_Pin_9) ? 0 : 1; /* active-low: low=pressed */

    if (key3_up) {
        if (key3_low) {
            debounce_cnt++;
            if (debounce_cnt >= 2) {  /* 2 * ~10ms = ~20ms stable confirmation */
                debounce_cnt = 0;
                key3_up = 0;
                return 1;             /* confirmed press event */
            }
        } else {
            debounce_cnt = 0;         /* glitch: reset counter */
        }
    } else {
        if (!key3_low) {
            key3_up = 1;              /* key released */
            debounce_cnt = 0;
        }
    }
    return 0;
}

/* ---- Increment Local Clock ---- */
static const uint8_t days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void Increment_LocalClock(void) {
    net_second++;
    if (net_second >= 60) {
        net_second = 0;
        net_minute++;
        if (net_minute >= 60) {
            net_minute = 0;
            net_hour++;
            if (net_hour >= 24) {
                net_hour = 0;
                
                // Day increment
                uint8_t dim = days_in_months[net_month - 1];
                if (net_month == 2) {
                    uint8_t is_leap = (net_year % 4 == 0 && (net_year % 100 != 0 || net_year % 400 == 0));
                    if (is_leap) dim = 29;
                }
                
                net_day++;
                if (net_day > dim) {
                    net_day = 1;
                    net_month++;
                    if (net_month > 12) {
                        net_month = 1;
                        net_year++;
                    }
                }
            }
        }
    }
    sprintf(current_net_time, "%04d-%02d-%02d %02d:%02d:%02d", net_year, net_month, net_day, net_hour, net_minute, net_second);
}

/* ---- Check And Send WeChat Alarm Alert ---- */
void Check_And_SendWeChatAlert(const char* event_type, const char* details) {
    if (!wifi_connected) return;
    if (system_uptime_s - last_wechat_alert_sec < 180 && last_wechat_alert_sec != 0) {
        return; // Cooldown of 3 minutes
    }
    last_wechat_alert_sec = system_uptime_s;
    ESP8266_SendWeChatAlert(SERVER_CHAN_KEY, event_type, details);
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
    IR_Init();
    LCD_Init();
    Theme_Init();

    LCD_Clear(theme_bg);

#if RUN_MODE_ALL || RUN_MODE_ENV_AI
    /* 初始化 BH1750 (软件 I2C) */
    bh1750_healthy = (BH1750_Init() == 0);
    /* 重新配置并启用 SD 卡 (MISO已移至PA11) */
    sd_healthy = (SD_Init() == SD_RESPONSE_NO_ERROR);
#endif

    TM1637_Init();

#if RUN_MODE_ALL || RUN_MODE_SECURITY
    /* 初始化 HC-SR04 超声波测距 */
    SR04_Init();
#endif
    
    /* 配置面包板 KEY3 按键 (PB9 上拉输入) 和蜂鸣器 (PB15 推挽输出) */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR;
    
    GPIOB->CRH &= ~0x000000F0;
    GPIOB->CRH |=  0x00000080;
    GPIOB->ODR |= GPIO_Pin_9;  // 开启上拉
    
    GPIOB->CRH &= ~0xF0000000;
    GPIOB->CRH |=  0x30000000;
    GPIOB->BRR = GPIO_Pin_15;  // 初始静音

#if RUN_MODE_ALL || RUN_MODE_ENV_AI
    if (sd_healthy) {
        /* 自动扫描 SD 卡空闲扇区以恢复数据记录
         * Fix: 限制扫描范围至 1000~4000 扇区（最多 3000 个），避免最坏情况
         * 下扫描 19000 扇区导致开机阻塞长达约 1.5 分钟。
         * 3000 个扇区 × 每扇区约 1ms（高速SPI约 100us，低速约 5ms）
         * 低速时序最坏情况约 15 秒，实际高速切换后约 0.3 秒，完全可接受。 */
        uint8_t temp_sector[512];
        log_sector_cursor = 1000;
        while (log_sector_cursor < 4000) {
            if (SD_ReadBlock(temp_sector, log_sector_cursor, 512) == SD_RESPONSE_NO_ERROR) {
                if ((temp_sector[0] == 'U' && temp_sector[1] == 'P' && temp_sector[2] == ':') ||
                    (temp_sector[0] == '2' && temp_sector[1] == '0' && temp_sector[2] == '2')) {
                    log_sector_cursor++;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }
#endif

#if RUN_MODE_ALL || RUN_MODE_ENV_AI
    /* 传感器初始化 */
    test_aht20_init_status  = AHT20_Init();
    aht20_healthy           = (test_aht20_init_status  == 0);
#endif

#if RUN_MODE_ALL || RUN_MODE_IOT
    /* ESP8266 IoT 初始化与联网 (SSID: xiaobo, PWD: 11111111) */
    if (ESP8266_Init() == 0) {
        wifi_hw_online = 1;
        if (ESP8266_ConnectWiFi("xiaobo", "11111111") == 0) {
            wifi_connected = 1;
            if (ESP8266_SyncNTP() == 0) {
                // 等待系统短时间稳定后，同步NTP服务器时间
                for (volatile int d = 0; d < 1440000; d++);
                char temp_time[24];
                if (ESP8266_GetNTPTime(temp_time) == 0) {
                    strcpy(current_net_time, temp_time);
                    int y, m, d, hh, mm, ss;
                    if (sscanf(current_net_time, "%d-%d-%d %d:%d:%d", &y, &m, &d, &hh, &mm, &ss) == 6) {
                        net_year = y;
                        net_month = m;
                        net_day = d;
                        net_hour = hh;
                        net_minute = mm;
                        net_second = ss;
                    }
                }
            }
            // 首次拉取室外天气
            float w_temp = 0.0f;
            char w_txt[16] = {0};
            if (ESP8266_GetWeather(&w_temp, w_txt) == 0) {
                outdoor_temp = w_temp;
                strcpy(outdoor_weather, w_txt);
            }
        }
    } else {
        wifi_hw_online = 0;
    }
#endif

#if RUN_MODE_ALL || RUN_MODE_ENV_AI
    /* AI & 日志初始化 (适配 MAX_ANOMALY_LOGS 为 7) */
    AI_Init(&my_detector, 0.2f, 100);
    Log_Init(&my_log_buffer);
#endif

    LED_Off(0);
    LED_Off(1);

#if RUN_MODE_ENV_AI
    current_page = 0;
#elif RUN_MODE_GAME
    current_page = 3;
    Game_Init();
#elif RUN_MODE_SECURITY
    current_page = 4;
#elif RUN_MODE_IOT
    current_page = 5;
#endif

    Display_Refresh(1);

    /* ---- 主循环 ---- */
    while (1) {
        /* 秒计数器 */
        uptime_cnt++;
        if (uptime_cnt >= 100) { 
            uptime_cnt = 0; 
            system_uptime_s++; 
            Increment_LocalClock();
            
            /* 每秒数据向 SD 卡缓存写入一次 */
#if RUN_MODE_ALL || RUN_MODE_ENV_AI
            if (sd_healthy) {
                char log_entry[65];
                int len = sprintf(log_entry, "%s T:%5.2fC H:%5.2f%% L:%5.1flx A:%d M:%d",
                                  current_net_time,
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
#endif
        }

        /* 按键扫描与缓存机制 (以防止 30ms 游戏循环漏按) */
        uint8_t key = KEY_Scan(0);
        static uint8_t game_key = 0;
        if (key != KEY_NONE) {
            game_key = key;
        }

        /* 扫描红外遥控接收 */
        uint8_t ir_cmd = 0;
        if (IR_GetData(&ir_cmd)) {
            // 通过串口1发送红外键值，方便调试与自定义修改
            char dbg_buf[32];
            sprintf(dbg_buf, "IR Received CMD: 0x%02X\r\n", ir_cmd);
            USART1_SendString(dbg_buf);

            // 1. 左右键翻页 (支持物理导航左/右键与数字键 7/9)
            if (ir_cmd == IR_NAV_LEFT || ir_cmd == IR_NUM_7) {
                current_page = (current_page + 5) % 6;
                if (current_page == 3) {
                    Game_Init();
                }
                Display_Refresh(1);
            } else if (ir_cmd == IR_NAV_RIGHT || ir_cmd == IR_NUM_9) {
                current_page = (current_page + 1) % 6;
                if (current_page == 3) {
                    Game_Init();
                }
                Display_Refresh(1);
            }
            // 2. OK键/1键/8键切换防盗布防模式 (支持物理导航OK键与数字键 1/8)
            else if (ir_cmd == IR_NAV_OK || ir_cmd == IR_NUM_1 || ir_cmd == IR_NUM_8) {
                security_alert_mode = !security_alert_mode;
                ui_dirty = 1;
                flash_cnt = 10; // 强制声光指示立即刷新
            }
            // 3. 3键/6键切换 LCD 主题 (数字键 3/6)
            else if (ir_cmd == IR_NUM_3 || ir_cmd == IR_NUM_6) {
                current_theme = !current_theme;
                Theme_Apply();
                Display_Refresh(1);
            }
            // 4. 向上键/5键映射至 Flappy Bird 游戏跳跃 (支持物理导航上键与数字键 5)
            else if (ir_cmd == IR_NAV_UP || ir_cmd == IR_NUM_5) {
                if (current_page == 3) {
                    game_key = KEY2_PRESS;
                }
            }
        }


        /* 扫描 KEY3 切换布防/撤防状态 */
        if (KEY3_Scan()) {
            security_alert_mode = !security_alert_mode;
            ui_dirty = 1;
            flash_cnt = 10; // 强制下一个循环立即更新 LED & 蜂鸣器
        }

        if (key == KEY1_PRESS) {
            if (current_page != 3) {
                /* KEY1 按键 (中间键 PB17): 硬件自愈重置 + 切换主题 */
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
        } else if (key == KEY_LEFT_PRESS) {
            /* 左键 (S2 / PB19): 向左循环切换页面 */
            current_page = (current_page + 5) % 6;
            if (current_page == 3) {
                Game_Init(); /* 切到游戏页面时初始化游戏数据 */
            }
            Display_Refresh(1);
        } else if (key == KEY_RIGHT_PRESS) {
            /* 右键 (S3 / PB20): 向右循环切换页面 */
            current_page = (current_page + 1) % 6;
            if (current_page == 3) {
                Game_Init(); /* 切到游戏页面时初始化游戏数据 */
            }
            Display_Refresh(1);
        }

        /* 1.5 秒传感器采样轮询 */
        loop_cnt++;
        if (loop_cnt == 130) {
#if RUN_MODE_ALL || RUN_MODE_ENV_AI
            /* 提前 200ms 触发 AHT20 温湿度测量以防阻塞 */
            if (aht20_healthy) {
                AHT20_StartMeasure();
            }
        }
        if (loop_cnt >= 150) {
            loop_cnt = 0;

#if RUN_MODE_ALL || RUN_MODE_ENV_AI
            /* AHT20 */
            if (!aht20_healthy) {
                if (AHT20_Init() == 0) { aht20_healthy = 1; test_aht20_init_status = 0; aht20_fail_cnt = 0; }
            }
            if (aht20_healthy) {
                float t, h;
                uint8_t res = AHT20_RetrieveData(&t, &h);
                if (res == 0) {
                    test_aht20_temp = t; test_aht20_humi = h; aht20_fail_cnt = 0;
                } else if (res == 2) {
                    /* 仍在忙，等待下次读取 */
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
#endif

#if RUN_MODE_ALL || RUN_MODE_IOT
            /* WiFi 周期性校准时钟与获取天气 / 自动检测重连 (仅在非游戏页面时执行以防游戏卡顿) */
            if (current_page != 3) {
                if (wifi_connected) {
                    wifi_hw_online = 1;
                    static uint8_t initial_sync_retry_cnt = 0;
                    uint8_t need_sync = 0;
                    
                    if (last_time_sync_sec == 0) {
                        initial_sync_retry_cnt++;
                        if (initial_sync_retry_cnt >= 3) { /* ~5 秒 */
                            initial_sync_retry_cnt = 0;
                            need_sync = 1;
                        }
                    } else if (system_uptime_s - last_time_sync_sec >= 600) {
                        need_sync = 1;
                    }
                    
                    if (need_sync) {
                        char temp_time[24];
                        uint8_t ntp_ok = 0;
                        uint8_t weather_ok = 0;
                        
                        /* 尝试同步 NTP 时钟 */
                        (void)ESP8266_SyncNTP();
                        for (volatile int d = 0; d < 720000; d++);
                        
                        if (ESP8266_GetNTPTime(temp_time) == 0) {
                            strcpy(current_net_time, temp_time);
                            int y, m, d_val, hh, mm, ss;
                            if (sscanf(current_net_time, "%d-%d-%d %d:%d:%d", &y, &m, &d_val, &hh, &mm, &ss) == 6) {
                                net_year = y;
                                net_month = m;
                                net_day = d_val;
                                net_hour = hh;
                                net_minute = mm;
                                net_second = ss;
                                ntp_ok = 1;
                            }
                        }
                        
                        /* 尝试拉取天气 (天气获取成功时会自动解析 HTTP 头部 Date 并同步时钟作为备用) */
                        float w_temp = 0.0f;
                        char w_txt[16] = {0};
                        if (ESP8266_GetWeather(&w_temp, w_txt) == 0) {
                            outdoor_temp = w_temp;
                            strcpy(outdoor_weather, w_txt);
                            weather_ok = 1;
                        }
                        
                        if (ntp_ok || weather_ok) {
                            last_time_sync_sec = system_uptime_s; /* 任意一个成功即认为本次校准完成 */
                        } else {
                            last_time_sync_sec = 0; /* 全部失败，下次循环继续尝试 */
                            /* 检查是否 WiFi 硬件断开或网络断开 */
                            if (ESP8266_IsHardwareOnline() == 0) {
                                wifi_hw_online = 0;
                                wifi_connected = 0;
                            } else if (ESP8266_IsConnected() == 0) {
                                wifi_connected = 0;
                            }
                        }
                    }
                } else {
                    static uint8_t wifi_check_cnt = 0;
                    static uint8_t wifi_reconnect_timer = 0;
                    wifi_check_cnt++;
                    if (wifi_check_cnt >= 3) { /* ~5 秒 */
                        wifi_check_cnt = 0;
                        if (ESP8266_IsHardwareOnline() == 1) {
                            wifi_hw_online = 1;
                            if (ESP8266_IsConnected() == 1) {
                                wifi_connected = 1;
                                wifi_reconnect_timer = 0;
                                last_time_sync_sec = system_uptime_s;
                                /* 同步 NTP 时间 */
                                if (ESP8266_SyncNTP() == 0) {
                                    for (volatile int d = 0; d < 720000; d++);
                                    char temp_time[24];
                                    if (ESP8266_GetNTPTime(temp_time) == 0) {
                                        strcpy(current_net_time, temp_time);
                                        int y, m, d, hh, mm, ss;
                                        if (sscanf(current_net_time, "%d-%d-%d %d:%d:%d", &y, &m, &d, &hh, &mm, &ss) == 6) {
                                            net_year = y; net_month = m; net_day = d;
                                            net_hour = hh; net_minute = mm; net_second = ss;
                                        }
                                    }
                                }
                            }
                            /* 获取天气 */
                            float w_temp = 0.0f;
                            char w_txt[16] = {0};
                            if (ESP8266_GetWeather(&w_temp, w_txt) == 0) {
                                outdoor_temp = w_temp;
                                strcpy(outdoor_weather, w_txt);
                            }
                        } else {
                            /* 硬件在线，但网络未连接，周期性触发连接指令 (每 30 秒 = 6 次检测) */
                            wifi_reconnect_timer++;
                            if (wifi_reconnect_timer >= 6) {
                                wifi_reconnect_timer = 0;
                                ESP8266_ConnectWiFi("xiaobo", "11111111");
                            }
                        }
                    } else {
                        wifi_hw_online = 0;
                        wifi_connected = 0;
                        wifi_reconnect_timer = 0;
                        /* 尝试重新初始化硬件并连接 */
                        if (ESP8266_Init() == 0) {
                            wifi_hw_online = 1;
                            ESP8266_ConnectWiFi("xiaobo", "11111111");
                        }
                    }
                }
            }
#endif

#if RUN_MODE_ALL || RUN_MODE_SECURITY
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

            /* 防盗入侵微信推送报警 */
            if (security_alert_mode && ultrasonic_healthy && test_ultrasonic_dist < 15.0f) {
                char details[32];
                sprintf(details, "Dist_%.1fcm", test_ultrasonic_dist);
                Check_And_SendWeChatAlert("Intruder_Alarm", details);
            }
#endif

#if RUN_MODE_ALL || RUN_MODE_ENV_AI
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

                if (current_ai_state != AI_STATE_ANOMALY && ns == AI_STATE_ANOMALY) {
                    Log_Add(&my_log_buffer, system_uptime_s,
                            my_detector.baseline_temp, filt, 0.0f);
                    
                    /* 微信推送温度突变警报 */
                    char details[32];
                    sprintf(details, "Temp_%.1fC_Dev_%.1fC", filt, dev);
                    Check_And_SendWeChatAlert("Temp_Anomaly", details);
                }
                current_ai_state = ns;
            }

            system_mode = (current_ai_state == AI_STATE_ANOMALY || !aht20_healthy);
#endif
            ui_dirty = 1;
        }

        /* UI 周期更新 */
        if (ui_dirty) { 
            ui_dirty = 0; 
            Display_Refresh(0); 
        }

        /* 游戏更新与刷新 (30ms 运行) */
#if RUN_MODE_ALL || RUN_MODE_GAME
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
#endif

        /* Beacon 心跳 */
        Draw_Beacon();

        /* LED与蜂鸣器综合状态机调度 (5Hz频率, 每100ms周期刷新) */
        if (wifi_connected) {
            LED_On(0); // WiFi连接成功时板载 LED PC13 常亮
        } else {
            LED_Off(0); // 未连接时熄灭
        }
        
        flash_cnt++;
        if (flash_cnt >= 10) { // 10 * 10ms = 100ms (5Hz 周期)
            flash_cnt = 0;
            
            if (security_alert_mode) {
                /* ==================== ARMED 布防模式 ==================== */
                if (!ultrasonic_healthy) {
                    // 超声波异常：LED2 闪烁，蜂鸣器关闭
                    LED_Toggle(1);
                    GPIOB->BRR = GPIO_Pin_15;
                } else {
                    float dist = test_ultrasonic_dist;
                    if (dist < 15.0f) {
                        // 报警级（<15cm）：LED2 闪烁，蜂鸣器常鸣
                        LED_Toggle(1);
                        GPIOB->BSRR = GPIO_Pin_15;
                    } else if (dist < 50.0f) {
                        // 警告级（15~50cm）：LED2 常亮，蜂鸣器关闭
                        LED_On(1);
                        GPIOB->BRR = GPIO_Pin_15;
                    } else {
                        // 安全级（>50cm）：LED2 熄灭，蜂鸣器关闭
                        LED_Off(1);
                        GPIOB->BRR = GPIO_Pin_15;
                    }
                }
            } else {
                /* ==================== DISARMED 撤防模式 ==================== */
                GPIOB->BRR = GPIO_Pin_15; // 撤防时蜂鸣器始终关闭
                
                if (!aht20_healthy || !bh1750_healthy || !sd_healthy) {
                    // 硬件故障：LED2 常亮
                    LED_On(1);
                } else if (current_ai_state == AI_STATE_ANOMALY) {
                    // 温度异常（偏差超过正负5C）：LED2 闪烁
                    LED_Toggle(1);
                } else if (current_ai_state == AI_STATE_LEARNING) {
                    // AI学习：LED2 常亮
                    LED_On(1);
                } else {
                    // 安全监控中：由 LED_ProcessBreathing() 呼吸控制，这里不强制修改
                }
            }
        }

        /* 呼吸灯平滑更新 (每 10ms 周期执行) */
        if (!security_alert_mode && aht20_healthy && bh1750_healthy && sd_healthy && current_ai_state == AI_STATE_NORMAL) {
            LED_ProcessBreathing();
        }

        Delay(72000);   /* ~10ms 循环节 */
    }
}

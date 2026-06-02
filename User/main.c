#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "lcd.h"
#include "ai_detect.h"
#include "anomaly_log.h"
#include "game.h"
#include "touch.h"
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
    LCD_FillCircle(x + 3, y + 11, 2, RED);
    LCD_DrawLine(x + 3, y + 3, x + 3, y + 9, RED);
}

/* 绘制像素水滴图标 */
static void Draw_DropIcon(uint16_t x, uint16_t y, uint16_t color) {
    LCD_DrawLine(x + 3, y, x, y + 6, color);
    LCD_DrawLine(x + 3, y, x + 6, y + 6, color);
    LCD_DrawCircle(x + 3, y + 7, 3, color);
    LCD_FillCircle(x + 3, y + 7, 1, BLUE);
}

/* 绘制气压表图标 */
static void Draw_GaugeIcon(uint16_t x, uint16_t y, uint16_t color) {
    LCD_DrawCircle(x + 6, y + 6, 6, color);
    LCD_DrawLine(x + 6, y + 6, x + 9, y + 3, CYAN);
    LCD_DrawPoint(x + 6, y + 6, color);
}

/* 绘制大脑 (AI) 节点图标 */
static void Draw_BrainIcon(uint16_t x, uint16_t y, uint16_t color) {
    LCD_DrawLine(x + 3, y + 1, x + 1, y + 6, GRAY);
    LCD_DrawLine(x + 3, y + 1, x + 7, y + 4, GRAY);
    LCD_DrawLine(x + 1, y + 6, x + 5, y + 10, GRAY);
    LCD_DrawLine(x + 7, y + 4, x + 5, y + 10, GRAY);
    LCD_DrawLine(x + 1, y + 6, x + 7, y + 4, GRAY);
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
    if (!aht20_healthy) return RED;
    if (current_ai_state == AI_STATE_ANOMALY) return RED;
    if (current_ai_state == AI_STATE_LEARNING) return BLUE;
    return DARK_GRAY;
}

/* ---- 绘制顶部状态栏（科技徽章风格） ---- */
static void Draw_TopBar(void) {
    uint16_t bg = TopBarBg();
    
    // 清空背景
    LCD_FillRect(0, 0, LCD_WIDTH, TOP_BAR_H, BLACK);
    
    // 霓虹底部双色线
    uint16_t line_color = (bg == RED) ? RED : ((bg == BLUE) ? BLUE : CYAN);
    LCD_DrawLine(0, TOP_BAR_H - 1, LCD_WIDTH - 1, TOP_BAR_H - 1, line_color);
    
    char buf[40];
    uint16_t text_color;
    if (!aht20_healthy) {
        strcpy(buf, " SENSOR ERROR ");
        text_color = RED;
    } else if (current_ai_state == AI_STATE_ANOMALY) {
        strcpy(buf, " ANOMALY ALARM ");
        text_color = RED;
    } else if (current_ai_state == AI_STATE_LEARNING) {
        strcpy(buf, " AI LEARNING ");
        text_color = BLUE;
    } else {
        strcpy(buf, " SYSTEM SAFE ");
        text_color = GREEN;
    }

    uint16_t text_len = (uint16_t)strlen(buf);
    uint16_t pill_w = text_len * 8 + 12;
    uint16_t pill_h = 16;
    uint16_t pill_x = (LCD_WIDTH - pill_w) / 2;
    uint16_t pill_y = (TOP_BAR_H - pill_h) / 2;
    
    LCD_DrawRect(pill_x, pill_y, pill_w, pill_h, text_color);
    Draw_CornerBrackets(pill_x, pill_y, pill_w, pill_h, 3, text_color);
    LCD_ShowString(pill_x + 6, pill_y + 1, buf, text_color, BLACK);
}

/* ---- 绘制副标题（带有科技几何装饰线） ---- */
static void Draw_Header(const char *title) {
    LCD_DrawLine(0, SEP_Y1, LCD_WIDTH - 1, SEP_Y1, GRAY);
    LCD_FillRect(0, SEP_Y1 + 1, LCD_WIDTH, SEP_Y2 - SEP_Y1 - 1, BLACK);
    
    // Left vertical cyan accent bar for premium look
    LCD_FillRect(8, TITLE_Y, 3, 16, CYAN);
    
    LCD_ShowString(16, TITLE_Y, title, CYAN, BLACK);
    
    // 右侧科技感短斜线装饰
    LCD_DrawLine(LCD_WIDTH - 60, TITLE_Y + 4, LCD_WIDTH - 24, TITLE_Y + 4, GRAY);
    LCD_DrawLine(LCD_WIDTH - 24, TITLE_Y + 4, LCD_WIDTH - 20, TITLE_Y, GRAY);
    
    LCD_DrawLine(0, SEP_Y2, LCD_WIDTH - 1, SEP_Y2, GRAY);
}

/* ---- 绘制底部翻页指示点（呼吸棱形） ---- */
static void Draw_PageDots(void) {
    uint16_t dots[4] = {210, 230, 250, 270};
    for (uint8_t i = 0; i < 4; i++) {
        if (i == current_page) {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 3, CYAN);
            LCD_DrawCircle(dots[i], PAGE_DOT_Y, 5, CYAN);
        } else {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 2, BLACK);
            LCD_DrawCircle(dots[i], PAGE_DOT_Y, 3, GRAY);
        }
    }
}

/* ---- 底部系统信息 ---- */
static void Draw_BottomInfo(void) {
    char buf[24];
    sprintf(buf, "SYS-UP: %5ds", system_uptime_s);
    LCD_FillRect(0, BOTTOM_Y, 150, 16, BLACK);
    LCD_ShowString(16, BOTTOM_Y, buf, GRAY, BLACK);

    sprintf(buf, "G-05 HUD");
    uint16_t x = LCD_WIDTH - (uint16_t)strlen(buf) * 8 - 16;
    LCD_FillRect(x, BOTTOM_Y, 80, 16, BLACK);
    LCD_ShowString(x, BOTTOM_Y, buf, GRAY, BLACK);
}

/* ---- Page 1: 偏差网格折线图（示波器风格） ---- */
static void Draw_DevChart(void) {
    uint16_t cx = 216, cy = 56, cw = 248, ch = 220;
    uint16_t zero_y = cy + ch / 2;
    
    // 绘制示波器点虚线网格
    Draw_DottedLine(cx + 6, zero_y - 45, cx + cw - 7, zero_y - 45, DARK_GRAY);  /* +3.0C */
    Draw_DottedLine(cx + 6, zero_y + 45, cx + cw - 7, zero_y + 45, DARK_GRAY);  /* -3.0C */
    
    for (uint16_t gx = cx + 30; gx < cx + cw - 10; gx += 38) {
        Draw_DottedLine(gx, cy + 10, gx, cy + ch - 10, DARK_GRAY);
    }
    
    Draw_DottedLine(cx + 6, zero_y, cx + cw - 7, zero_y, GRAY);       /* 零偏差线 */
    LCD_DrawLine(cx + 6, cy + 10, cx + 6, cy + ch - 10, GRAY);         /* 左轴 */
    LCD_DrawLine(cx + cw - 6, cy + 10, cx + cw - 6, cy + ch - 10, GRAY); /* 右轴 */

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

        LCD_DrawLine(x1, (uint16_t)y1_val, x2, (uint16_t)y2_val, CYAN);
        LCD_FillCircle(x2, (uint16_t)y2_val, 1, CYAN); // 绘制数据接点
    }
}

/* ============================================================
 * Display_Refresh — 主 UI 绘制函数
 * ============================================================ */
void Display_Refresh(uint8_t force_refresh) {
    char buf[40];

    /* 切页 → 全屏清黑 */
    if (current_page != last_page || force_refresh) {
        LCD_Clear(BLACK);
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
    }

    /* ================================================================
     * PAGE 0: 实时监测 (HUD 两栏卡片网格布局)
     * ================================================================ */
    if (current_page == 0) {
        
        // ------------------ 左侧栏 ------------------
        
        // Card 1: Temperature Raw (Y: 56..156, height 100)
        if (force_refresh) {
            LCD_FillRect(16, 56, 216, 100, DARK_GRAY);
            LCD_DrawRect(16, 56, 216, 100, GRAY);
            Draw_CornerBrackets(16, 56, 216, 100, 6, CYAN);
            Draw_TempIcon(28, 66, CYAN);
            LCD_ShowString(42, 66, "TEMP RAW:", CYAN, DARK_GRAY);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", test_aht20_temp);
            LCD_ShowString(28, 90, buf, WHITE, DARK_GRAY);
            
            float t = test_aht20_temp;
            if (t < 10.0f) t = 10.0f;
            if (t > 40.0f) t = 40.0f;
            uint16_t active_segs = (uint16_t)((t - 10.0f) * 12.0f / 30.0f);
            uint16_t bar_color = (t > 30.0f || t < 15.0f) ? RED : CYAN;
            Draw_SegmentedBar(36, 122, active_segs, 12, bar_color, BLACK);
        } else {
            LCD_ShowString(28, 90, "[ERROR]", RED, DARK_GRAY);
            Draw_SegmentedBar(36, 122, 0, 12, RED, BLACK);
        }

        // Card 2: Humidity (Y: 176..276, height 100)
        if (force_refresh) {
            LCD_FillRect(16, 176, 216, 100, DARK_GRAY);
            LCD_DrawRect(16, 176, 216, 100, GRAY);
            Draw_CornerBrackets(16, 176, 216, 100, 6, CYAN);
            Draw_DropIcon(28, 186, CYAN);
            LCD_ShowString(40, 186, "HUMIDITY:", CYAN, DARK_GRAY);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f %%", test_aht20_humi);
            LCD_ShowString(28, 210, buf, WHITE, DARK_GRAY);
            
            float h = test_aht20_humi;
            if (h < 0.0f) h = 0.0f;
            if (h > 100.0f) h = 100.0f;
            uint16_t active_segs = (uint16_t)(h * 12.0f / 100.0f);
            Draw_SegmentedBar(36, 242, active_segs, 12, CYAN, BLACK);
        } else {
            LCD_ShowString(28, 210, "[ERROR]", RED, DARK_GRAY);
            Draw_SegmentedBar(36, 242, 0, 12, RED, BLACK);
        }

        // ------------------ 右侧栏 ------------------
        
        // Card 3: AI Filtered Temp (Y: 56..156, height 100)
        if (force_refresh) {
            LCD_FillRect(248, 56, 216, 100, DARK_GRAY);
            LCD_DrawRect(248, 56, 216, 100, GRAY);
            Draw_CornerBrackets(248, 56, 216, 100, 6, CYAN);
            Draw_BrainIcon(260, 66, CYAN);
            LCD_ShowString(274, 66, "AI FILTERED:", CYAN, DARK_GRAY);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", test_filtered_temp);
            LCD_ShowString(260, 90, buf, WHITE, DARK_GRAY);
            LCD_ShowString(260, 122, "(EMA alpha=0.2)", GRAY, DARK_GRAY);
        } else {
            LCD_ShowString(260, 90, "[ERROR]", RED, DARK_GRAY);
        }

        // Card 4: System Health Info (Y: 176..276, height 100)
        if (force_refresh) {
            LCD_FillRect(248, 176, 216, 100, DARK_GRAY);
            LCD_DrawRect(248, 176, 216, 100, GRAY);
            Draw_CornerBrackets(248, 176, 216, 100, 6, CYAN);
            LCD_ShowString(260, 186, "SYSTEM HEALTH:", CYAN, DARK_GRAY);
            LCD_ShowString(260, 208, "AHT20  :", GRAY, DARK_GRAY);
            LCD_ShowString(260, 230, "AI STATE:", CYAN, DARK_GRAY);
        }
        
        LCD_ShowString(332, 208, aht20_healthy ? "ONLINE " : "OFFLINE", aht20_healthy ? GREEN : RED, DARK_GRAY);
        LCD_FillCircle(322, 214, 3, aht20_healthy ? GREEN : RED);
        
        if (!aht20_healthy) {
            LCD_ShowString(260, 248, "FAULT/SENSOR ERR", RED, DARK_GRAY);
        } else if (current_ai_state == AI_STATE_LEARNING) {
            uint8_t pct = (my_detector.learning_samples * 100) / 100;
            sprintf(buf, "LEARNING (%3d%%)", pct);
            LCD_ShowString(260, 248, buf, BLUE, DARK_GRAY);
        } else if (current_ai_state == AI_STATE_NORMAL) {
            LCD_ShowString(260, 248, "MONITOR NORMAL  ", GREEN, DARK_GRAY);
        } else {
            LCD_ShowString(260, 248, "ANOMALY ALARM!! ", RED, DARK_GRAY);
        }
    }

    /* ================================================================
     * PAGE 1: AI自适应基准学习 (左栏 HUD 卡片 + 右栏趋势图)
     * ================================================================ */
    else if (current_page == 1) {
        
        // ------------------ 左侧诊断卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(16, 56, 184, 220, DARK_GRAY);
            LCD_DrawRect(16, 56, 184, 220, GRAY);
            Draw_CornerBrackets(16, 56, 184, 220, 6, CYAN);
            
            Draw_TempIcon(24, 62, GREEN);
            LCD_ShowString(38, 62, "Baseline Temp:", CYAN, DARK_GRAY);
            
            Draw_BrainIcon(24, 96, YELLOW);
            LCD_ShowString(38, 96, "Filtered Temp:", CYAN, DARK_GRAY);
            
            LCD_ShowString(24, 130, "Deviation:", CYAN, DARK_GRAY);
            LCD_ShowString(24, 204, "AI Progress:", CYAN, DARK_GRAY);
        }
        
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", my_detector.baseline_temp);
            LCD_ShowString(24, 78, buf, GREEN, DARK_GRAY);
            
            sprintf(buf, "%5.2f C", test_filtered_temp);
            LCD_ShowString(24, 112, buf, YELLOW, DARK_GRAY);
            
            float dev = test_filtered_temp - my_detector.baseline_temp;
            sprintf(buf, "%+5.2f C", dev);
            uint16_t dc = GREEN;
            if (dev > 3.0f || dev < -3.0f) dc = RED;
            else if (dev > 1.5f || dev < -1.5f) dc = YELLOW;
            LCD_ShowString(24, 146, buf, dc, DARK_GRAY);
            
            // 偏差指示小彩条 (X:24..176, width 152)
            LCD_FillRect(24, 168, 152, 6, BLACK);
            LCD_DrawRect(24, 168, 152, 6, GRAY);
            LCD_DrawLine(100, 166, 100, 171, GRAY); // 中心零刻度
            
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
            LCD_ShowString(24, 78, "[ERROR]", RED, DARK_GRAY);
            LCD_ShowString(24, 112, "[ERROR]", RED, DARK_GRAY);
            LCD_ShowString(24, 146, "[ERROR]", RED, DARK_GRAY);
        }
        
        // 学习样本进度条
        sprintf(buf, "%3d / %d", my_detector.learning_samples, my_detector.max_learning_samples);
        LCD_ShowString(24, 222, buf, WHITE, DARK_GRAY);
        
        {
            uint32_t cur = my_detector.learning_samples;
            uint32_t max = my_detector.max_learning_samples;
            if (cur > max) cur = max;
            uint16_t active_segs = (uint16_t)(cur * 10 / max);
            uint16_t pc = my_detector.is_learning_done ? GREEN : BLUE;
            Draw_SegmentedBar(24, 244, active_segs, 10, pc, BLACK);
        }

        // ------------------ 右侧趋势图卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(216, 56, 248, 220, DARK_GRAY);
            LCD_DrawRect(216, 56, 248, 220, GRAY);
            Draw_CornerBrackets(216, 56, 248, 220, 6, GRAY);
            LCD_ShowString(228, 62, "Deviation Trend (+/-5 C)", CYAN, DARK_GRAY);
        }
        Draw_DevChart();
    }

    /* ================================================================
     * PAGE 2: 历史日志 (科技感时空节点布局)
     * ================================================================ */
    else if (current_page == 2) {
        if (my_log_buffer.count == 0) {
            if (force_refresh) {
                LCD_ShowString(120, 150, "NO ANOMALY LOGS RECORDED", GRAY, BLACK);
            }
        } else {
            /* 1. 先绘制点虚线时间轴垂直线 */
            if (force_refresh) {
                Draw_DottedLine(24, 56, 24, 276, GRAY);
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
                    LCD_DrawLine(24, row_y + 24, 42, row_y + 24, GRAY);
                    LCD_FillCircle(24, row_y + 24, 2, RED);
                    LCD_DrawCircle(24, row_y + 24, 5, RED);

                    /* 右侧日志卡片背景与细灰色边框，配合红角标 */
                    LCD_FillRect(42, row_y, 422, 48, DARK_GRAY);
                    LCD_DrawRect(42, row_y, 422, 48, GRAY);
                    Draw_CornerBrackets(42, row_y, 422, 48, 4, RED);

                    /* 卡片内部信息 */
                    sprintf(buf, "#%d [TIME: %d s]", i + 1, ev->timestamp_s);
                    LCD_ShowString(52, row_y + 6, buf, YELLOW, DARK_GRAY);

                    sprintf(buf, "Base: %5.2f C -> Curr: %5.2f C", ev->baseline_temp, ev->current_temp);
                    LCD_ShowString(52, row_y + 26, buf, RED, DARK_GRAY);


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
        uint16_t fg = (bg == RED) ? RED : ((bg == BLUE) ? BLUE : GREEN);
        LCD_ShowString(LCD_WIDTH - 16, 4, s, fg, BLACK);
        idx = (idx + 1) & 3;
    }
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
    LCD_Clear(BLACK);
    TP_Init();

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
        }

        /* 按键扫描与缓存机制 (以防止 30ms 游戏循环漏按) */
        uint8_t key = KEY_Scan(0);
        static uint8_t game_key = 0;
        if (key != KEY_NONE) {
            game_key = key;
        }

        /* 触摸屏扫描与手势/点击处理 */
        tp_dev.scan(0);
        
        static uint8_t touch_active = 0;
        static uint16_t touch_start_x = 0;
        static uint16_t touch_start_y = 0;
        static uint8_t swipe_triggered = 0;
        static uint8_t tap_triggered = 0;
        
        if (tp_dev.sta & TP_PRES_DOWN) {
            if (tp_dev.x < 480 && tp_dev.y < 320) {
                if (!touch_active) {
                    touch_active = 1;
                    touch_start_x = tp_dev.x;
                    touch_start_y = tp_dev.y;
                    swipe_triggered = 0;
                    tap_triggered = 0;
                } else if (!swipe_triggered) {
                    int dx = (int)tp_dev.x - (int)touch_start_x;
                    int dy = (int)tp_dev.y - (int)touch_start_y;
                    
                    // 滑动检测阈值: 80 像素
                    if (dx > 80 && abs(dy) < 60) {
                        // 向右滑动: 切换到上一个页面
                        current_page = (current_page + 3) % 4;
                        if (current_page == 3) Game_Init();
                        Display_Refresh(1);
                        swipe_triggered = 1;
                    } else if (dx < -80 && abs(dy) < 60) {
                        // 向左滑动: 切换到下一个页面
                        current_page = (current_page + 1) % 4;
                        if (current_page == 3) Game_Init();
                        Display_Refresh(1);
                        swipe_triggered = 1;
                    }
                }
                
                // 如果在游戏页面 (Page 3) 且未触发滑动，则检测 Tap 点击
                if (current_page == 3 && !swipe_triggered && !tap_triggered) {
                    // 点击游戏画面区域 (X: 20..460, Y: 56..286)
                    if (tp_dev.x >= 20 && tp_dev.x <= 460 && tp_dev.y >= 56 && tp_dev.y <= 286) {
                        game_key = 2; // 触发小鸟拍击翅膀 (KEY2)
                        tap_triggered = 1;
                    }
                }
            }
        } else {
            touch_active = 0;
            swipe_triggered = 0;
            tap_triggered = 0;
        }


        if (key == KEY1_PRESS) {
            /* KEY1 按键: 循环切换页面 (0 -> 1 -> 2 -> 3 -> 0) */
            current_page = (current_page + 1) % 4;
            if (current_page == 3) {
                Game_Init(); /* 切到游戏页面时初始化游戏数据 */
            }
            Display_Refresh(1);
        } else if (key == KEY2_PRESS) {
            if (current_page != 3) {
                /* KEY2 按键 (其他常规页面模式): 硬件自愈重置 */
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

        /* LED 驱动调度 (利用板载 PA1 LED3 & PA2 LED4 指示AI状态及闪烁告警) */
        if (system_mode == 0) {
            /* 正常模式：LED1 (PC13) 常灭，LED2 (PA0) 呼吸渐变 */
            LED_Off(0);
            LED_ProcessBreathing();
            
            /* LED3 & LED4 指示 AI 学习/监控状态 */
            if (current_ai_state == AI_STATE_LEARNING) {
                LED_On(2);  /* 学习中：LED3 (PA1) 亮 */
                LED_Off(3); /* LED4 (PA2) 灭 */
            } else {
                LED_Off(2); /* 学习完成：LED3 (PA1) 灭 */
                LED_On(3);  /* 监控中：LED4 (PA2) 亮 */
            }
        } else {
            /* 异常模式：LED2 (PA0) 强制熄灭，LED1 (PC13) 及 LED3/LED4 高频 (5Hz) 交替闪烁 */
            LED_Off(1);
            flash_cnt++;
            if (flash_cnt >= 10) { // 10 * 10ms = 100ms (5Hz 周期性闪烁)
                flash_cnt = 0;
                LED_Toggle(0);
                
                /* LED3 与 LED4 进行红蓝警灯式交替闪烁 */
                static uint8_t alt_state = 0;
                alt_state = !alt_state;
                if (alt_state) {
                    LED_On(2);
                    LED_Off(3);
                } else {
                    LED_Off(2);
                    LED_On(3);
                }
            }
        }

        Delay(72000);   /* ~10ms 循环节 */
    }
}

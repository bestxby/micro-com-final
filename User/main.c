#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "bmp280.h"
#include "lcd.h"
#include "ai_detect.h"
#include "anomaly_log.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * 全局变量 (传感器 / AI / 系统状态)
 * ============================================================ */
volatile float test_aht20_temp   = 0.0f;
volatile float test_aht20_humi   = 0.0f;
volatile float test_bmp280_temp  = 0.0f;
volatile float test_bmp280_press = 0.0f;

volatile uint8_t test_aht20_init_status  = 1;
volatile uint8_t test_bmp280_init_status = 1;

volatile uint8_t aht20_healthy  = 1;
volatile uint8_t bmp280_healthy = 1;

uint8_t aht20_fail_cnt  = 0;
uint8_t bmp280_fail_cnt = 0;

AI_Detector     my_detector;
AnomalyLogBuffer my_log_buffer;
volatile float  test_filtered_temp  = 0.0f;
volatile uint8_t current_ai_state   = AI_STATE_LEARNING;

float dev_history[24] = {0.0f};    /* 24 个采样点 → 偏差折线图 */

volatile uint32_t system_uptime_s = 0;
volatile uint8_t  system_mode     = 0;    /* 0=正常, 1=异常 */
volatile uint8_t  current_page    = 0;    /* 0/1/2 */
volatile uint8_t  last_page       = 99;

/* ============================================================
 * 布局常量 (320×480 竖屏)
 * ============================================================ */
#define TOP_BAR_H      28
#define SEP_Y1         29
#define TITLE_Y        34
#define SEP_Y2         54
#define CONTENT_Y      60
#define ROW_H          36
#define BOTTOM_Y       448
#define PAGE_DOT_Y     465

/* ============================================================
 * 内部辅助
 * ============================================================ */
void Delay(__IO uint32_t nCount) {
    for (; nCount != 0; nCount--);
}

/* ---- 获取当前顶部状态栏背景色 ---- */
static uint16_t TopBarBg(void) {
    if (!aht20_healthy || !bmp280_healthy) return RED;
    if (current_ai_state == AI_STATE_ANOMALY) return RED;
    if (current_ai_state == AI_STATE_LEARNING) return BLUE;
    return 0x1A;  /* 正常: 深蓝 */
}

/* ---- 绘制顶部状态栏 ---- */
static void Draw_TopBar(void) {
    uint16_t bg = TopBarBg();
    LCD_FillRect(0, 0, LCD_WIDTH, TOP_BAR_H, bg);

    char buf[32];
    if (!aht20_healthy || !bmp280_healthy)
        strcpy(buf, "!! SENSOR DISCONNECTED !!");
    else if (current_ai_state == AI_STATE_ANOMALY)
        strcpy(buf, "!! ANOMALY DETECTED !!");
    else if (current_ai_state == AI_STATE_LEARNING)
        strcpy(buf, "** LEARNING BASELINE **");
    else
        strcpy(buf, "** SYSTEM NORMAL **");

    uint16_t fg = (bg == RED || bg == BLUE) ? WHITE : GREEN;
    uint16_t x  = (LCD_WIDTH - (uint16_t)strlen(buf) * 8) / 2;
    LCD_ShowString(x, 6, buf, fg, bg);
}

/* ---- 绘制副标题 & 分隔线 ---- */
static void Draw_Header(const char *title) {
    LCD_DrawLine(0, SEP_Y1, LCD_WIDTH - 1, SEP_Y1, GRAY);   /* 上分隔线 */
    LCD_FillRect(0, SEP_Y1 + 1, LCD_WIDTH, TITLE_Y - SEP_Y1 - 1, BLACK);
    LCD_ShowString(12, TITLE_Y, title, CYAN, BLACK);
    LCD_DrawLine(0, SEP_Y2, LCD_WIDTH - 1, SEP_Y2, GRAY);   /* 下分隔线 */
}

/* ---- 绘制底部翻页指示点 ---- */
static void Draw_PageDots(void) {
    uint16_t dots[3] = {140, 160, 180};
    for (uint8_t i = 0; i < 3; i++) {
        if (i == current_page) {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 4, CYAN);
        } else {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 4, BLACK);
            LCD_DrawCircle(dots[i], PAGE_DOT_Y, 4, GRAY);
        }
    }
}

/* ---- 格式化浮点字符串 (定宽 6 字符) ---- */
static void PutFloat(uint16_t x, uint16_t y, float val, uint16_t fc, uint16_t bc) {
    char buf[12];
    sprintf(buf, "%6.2f", val);
    LCD_ShowString(x, y, buf, fc, bc);
}

/* ---- Page 1: 偏差折线图 (24 个采样点, 高度 120px) ---- */
static void Draw_DevChart(void) {
    uint16_t cx = 20, cy = 290, cw = 280, ch = 130;
    LCD_FillRect(cx, cy, cw, ch, BLACK);
    LCD_DrawRect(cx, cy, cw, ch, DARK_GRAY);

    /* 零线 */
    uint16_t zero_y = cy + ch / 2;
    LCD_DrawLine(cx, zero_y, cx + cw - 1, zero_y, DARK_GRAY);

    /* ±5°C 标线 */
    uint16_t p5_y  = zero_y - ch / 2 + 12;
    uint16_t m5_y  = zero_y + ch / 2 - 12;
    LCD_DrawLine(cx, p5_y, cx + cw - 1, p5_y, 0x2104);
    LCD_DrawLine(cx, m5_y, cx + cw - 1, m5_y, 0x2104);

    /* 折线 */
    float scale = (float)(ch / 2 - 12) / 5.0f;   /* 像素/°C */
    for (uint8_t i = 0; i < 23; i++) {
        uint16_t x1 = cx + i * (cw / 23);
        uint16_t x2 = cx + (i + 1) * (cw / 23);
        float y1_val = (float)zero_y - dev_history[i]   * scale;
        float y2_val = (float)zero_y - dev_history[i+1] * scale;

        if (y1_val < cy + 4) y1_val = cy + 4;
        if (y1_val > cy + ch - 4) y1_val = cy + ch - 4;
        if (y2_val < cy + 4) y2_val = cy + 4;
        if (y2_val > cy + ch - 4) y2_val = cy + ch - 4;

        LCD_DrawLine(x1, (uint16_t)y1_val, x2, (uint16_t)y2_val, YELLOW);
    }
}

/* ---- 底部系统信息 ---- */
static void Draw_BottomInfo(void) {
    char buf[24];
    sprintf(buf, "UPTIME: %5d s", system_uptime_s);
    LCD_FillRect(0, BOTTOM_Y, LCD_WIDTH, 16, BLACK);
    LCD_ShowString(4, BOTTOM_Y, buf, GRAY, BLACK);

    sprintf(buf, "GROUP 05");
    uint16_t x = LCD_WIDTH - (uint16_t)strlen(buf) * 8 - 4;
    LCD_ShowString(x, BOTTOM_Y, buf, GRAY, BLACK);
}

/* ============================================================
 * Display_Refresh — 主 UI 绘制函数
 * ============================================================ */
void Display_Refresh(uint8_t force_refresh) {
    char buf[32];

    /* 切页 → 全屏清黑 */
    if (current_page != last_page || force_refresh) {
        LCD_Clear(BLACK);
        last_page = current_page;
        force_refresh = 1;
    }

    /* ---- 顶部状态栏 (每帧刷新, 状态可能变化) ---- */
    Draw_TopBar();

    /* ---- 标题 (仅 force_refresh) ---- */
    if (force_refresh) {
        if (current_page == 0)
            Draw_Header("[1] REAL-TIME MONITOR");
        else if (current_page == 1)
            Draw_Header("[2] AI BASELINE LEARN");
        else
            Draw_Header("[3] HISTORY LOG");
    }

    uint16_t y;

    /* ================================================================
     * PAGE 0: 实时监测
     * ================================================================ */
    if (current_page == 0) {
        y = CONTENT_Y + 10;

        /* 大号温度 */
        if (force_refresh) {
            LCD_ShowString(20, y, "TEMPERATURE", GRAY, BLACK);
            LCD_DrawLine(20, y + 48, 200, y + 48, DARK_GRAY);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", test_aht20_temp);
            LCD_ShowString(20, y + 18, buf, WHITE, BLACK);
            /* 简易温度计条 */
            float t = test_aht20_temp;
            if (t < 10) t = 10; if (t > 40) t = 40;
            uint16_t t_w = (uint16_t)((t - 10) * 6);
            LCD_FillRect(20, y + 52, 180, 8, DARK_GRAY);
            LCD_FillRect(20, y + 52, t_w, 8, (t > 30) ? RED : GREEN);
        } else {
            LCD_ShowString(20, y + 18, "[ERROR]", RED, BLACK);
        }

        y += 76;
        if (force_refresh) {
            LCD_DrawLine(10, y, LCD_WIDTH - 10, y, DARK_GRAY);
        }
        y += 8;

        /* 湿度 & 气压 双栏 */
        if (force_refresh) {
            LCD_ShowString(20, y, "HUMIDITY", GRAY, BLACK);
            LCD_ShowString(170, y, "PRESSURE", GRAY, BLACK);
        }
        y += 20;
        if (aht20_healthy) {
            sprintf(buf, "%5.2f %%", test_aht20_humi);
            LCD_ShowString(20, y, buf, WHITE, BLACK);
        } else {
            LCD_ShowString(20, y, "[ERROR]", RED, BLACK);
        }
        if (bmp280_healthy) {
            sprintf(buf, "%6.0f Pa", test_bmp280_press);
            LCD_ShowString(170, y, buf, WHITE, BLACK);
        } else {
            LCD_ShowString(170, y, "[ERROR]", RED, BLACK);
        }

        y += 36;
        if (force_refresh) {
            LCD_DrawLine(10, y, LCD_WIDTH - 10, y, DARK_GRAY);
        }
        y += 8;

        /* BMP280 温度 & 系统状态 */
        if (force_refresh) {
            LCD_ShowString(20, y, "BMP TEMP", GRAY, BLACK);
            LCD_ShowString(170, y, "STATUS", GRAY, BLACK);
        }
        y += 20;
        if (bmp280_healthy) {
            sprintf(buf, "%5.2f C", test_bmp280_temp);
            LCD_ShowString(20, y, buf, WHITE, BLACK);
        } else {
            LCD_ShowString(20, y, "[ERROR]", RED, BLACK);
        }
        /* 系统状态标签 */
        {
            uint16_t sx = 170, sy = y;
            if (!aht20_healthy || !bmp280_healthy) {
                LCD_ShowString(sx, sy, "FAULT", RED, BLACK);
            } else if (current_ai_state == AI_STATE_LEARNING) {
                uint8_t pct = (my_detector.learning_samples * 100) / 100;
                sprintf(buf, "LEARN %3d%%", pct);
                LCD_ShowString(sx, sy, buf, BLUE, BLACK);
            } else if (current_ai_state == AI_STATE_NORMAL) {
                LCD_ShowString(sx, sy, "NORMAL", GREEN, BLACK);
            } else {
                LCD_ShowString(sx, sy, "ANOMALY", RED, BLACK);
            }
        }

        y += 36;
        if (force_refresh) {
            LCD_DrawLine(10, y, LCD_WIDTH - 10, y, DARK_GRAY);
        }
        y += 8;

        /* AI 滤波温度 */
        if (force_refresh)
            LCD_ShowString(20, y, "AI FILTERED TEMP", GRAY, BLACK);
        y += 20;
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C  (EMA a=0.2)", test_filtered_temp);
            LCD_ShowString(20, y, buf, CYAN, BLACK);
        } else {
            LCD_ShowString(20, y, "--", RED, BLACK);
        }

        y += 36;
        if (force_refresh) {
            LCD_DrawLine(10, y, LCD_WIDTH - 10, y, DARK_GRAY);
        }
        y += 8;

        /* 健康度指示 */
        if (force_refresh)
            LCD_ShowString(20, y, "SENSOR HEALTH", GRAY, BLACK);
        y += 20;
        LCD_ShowString(20, y, "AHT20 :", GRAY, BLACK);
        LCD_ShowString(76, y, aht20_healthy  ? "ONLINE" : "OFFLINE",
                       aht20_healthy  ? GREEN : RED, BLACK);
        LCD_ShowString(160, y, "BMP280:", GRAY, BLACK);
        LCD_ShowString(216, y, bmp280_healthy ? "ONLINE" : "OFFLINE",
                       bmp280_healthy ? GREEN : RED, BLACK);
    }

    /* ================================================================
     * PAGE 1: AI 基准学习
     * ================================================================ */
    else if (current_page == 1) {
        y = CONTENT_Y;

        if (force_refresh) {
            LCD_ShowString(16, y, "Baseline Temp:", GRAY, BLACK);
            LCD_ShowString(16, y + 18, "Filtered Temp:", GRAY, BLACK);
            LCD_ShowString(16, y + 36, "Deviation   :", GRAY, BLACK);
        }
        y += 4;
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", my_detector.baseline_temp);
            LCD_ShowString(120, y, buf, GREEN, BLACK);
            sprintf(buf, "%5.2f C", test_filtered_temp);
            LCD_ShowString(120, y + 18, buf, CYAN, BLACK);
            float dev = test_filtered_temp - my_detector.baseline_temp;
            sprintf(buf, "%+5.2f C", dev);
            uint16_t dc = GREEN;
            if (dev > 3.0f || dev < -3.0f) dc = RED;
            else if (dev > 1.5f || dev < -1.5f) dc = YELLOW;
            LCD_ShowString(120, y + 36, buf, dc, BLACK);
        } else {
            LCD_ShowString(120, y, "[ERROR]", RED, BLACK);
            LCD_ShowString(120, y + 18, "[ERROR]", RED, BLACK);
            LCD_ShowString(120, y + 36, "[ERROR]", RED, BLACK);
        }

        /* 偏差条 */
        y += 64;
        if (force_refresh) {
            LCD_ShowString(16, y, "Bias Bar (+/-5 C)", GRAY, BLACK);
        }
        y += 18;
        {
            LCD_DrawRect(20, y, 280, 16, DARK_GRAY);
            LCD_DrawLine(160, y - 2, 160, y + 17, GRAY);  /* 0 线 */

            float dev = test_filtered_temp - my_detector.baseline_temp;
            if (dev >  5.0f) dev =  5.0f;
            if (dev < -5.0f) dev = -5.0f;
            int16_t bar = (int16_t)(dev * 28.0f);  /* 5°C = 140px */
            if (bar > 0) {
                LCD_FillRect(160, y + 2, bar, 12,
                    (dev > 3) ? RED : (dev > 1.5f) ? YELLOW : GREEN);
                LCD_FillRect(160 + bar, y + 2, 140 - bar, 12, BLACK);
                LCD_FillRect(20, y + 2, 140, 12, BLACK);
            } else if (bar < 0) {
                LCD_FillRect(160 + bar, y + 2, -bar, 12,
                    (dev < -3) ? RED : (dev < -1.5f) ? YELLOW : GREEN);
                LCD_FillRect(20, y + 2, 140 + bar, 12, BLACK);
                LCD_FillRect(160, y + 2, 140, 12, BLACK);
            } else {
                LCD_FillRect(20, y + 2, 280, 12, BLACK);
            }
        }

        /* 学习进度 */
        y += 28;
        if (force_refresh)
            LCD_ShowString(16, y, "Learning Progress", GRAY, BLACK);
        y += 18;
        {
            LCD_DrawRect(20, y, 280, 14, DARK_GRAY);
            uint32_t cur = my_detector.learning_samples;
            uint32_t max = my_detector.max_learning_samples;
            if (cur > max) cur = max;
            uint16_t pw = (uint16_t)(cur * 276 / max);
            uint16_t pc = my_detector.is_learning_done ? GREEN : BLUE;
            if (pw > 0) LCD_FillRect(22, y + 2, pw, 10, pc);
            if (pw < 276) LCD_FillRect(22 + pw, y + 2, 276 - pw, 10, BLACK);
        }
        y += 20;
        sprintf(buf, "%3d / %d samples", my_detector.learning_samples,
                my_detector.max_learning_samples);
        LCD_ShowString(80, y, buf, YELLOW, BLACK);

        /* 偏差折线图 */
        Draw_DevChart();
    }

    /* ================================================================
     * PAGE 2: 历史日志
     * ================================================================ */
    else {
        if (my_log_buffer.count == 0) {
            LCD_ShowString(40, 220, "NO ANOMALY EVENTS RECORDED", GRAY, BLACK);
        } else {
            uint8_t start = (my_log_buffer.head + MAX_ANOMALY_LOGS - my_log_buffer.count) % MAX_ANOMALY_LOGS;
            for (uint8_t i = 0; i < my_log_buffer.count; i++) {
                uint8_t idx = (start + i) % MAX_ANOMALY_LOGS;
                AnomalyEvent *ev = &my_log_buffer.logs[idx];
                y = CONTENT_Y + i * 72;

                /* 卡片背景 */
                LCD_DrawRect(14, y, LCD_WIDTH - 28, 66, DARK_GRAY);

                sprintf(buf, "#%d  UPTIME: %5d s", i + 1, ev->timestamp_s);
                LCD_ShowString(20, y + 4, buf, YELLOW, BLACK);

                sprintf(buf, "Base: %5.2f C    Curr: %5.2f C",
                        ev->baseline_temp, ev->current_temp);
                LCD_ShowString(20, y + 24, buf, RED, BLACK);

                sprintf(buf, "Press: %6.0f Pa", ev->current_press);
                LCD_ShowString(20, y + 44, buf, GRAY, BLACK);
            }
        }
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
        uint16_t fg = (bg == RED || bg == BLUE) ? WHITE : GREEN;
        LCD_ShowString(LCD_WIDTH - 16, 6, s, fg, bg);
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

    /* 传感器初始化 */
    test_aht20_init_status  = AHT20_Init();
    aht20_healthy           = (test_aht20_init_status  == 0);
    test_bmp280_init_status = BMP280_Init();
    bmp280_healthy          = (test_bmp280_init_status == 0);

    /* AI & 日志初始化 */
    AI_Init(&my_detector, 0.2f, 100);
    Log_Init(&my_log_buffer);

    LED_Off(0);
    LED_Off(1);

    Display_Refresh(1);

    /* ---- 主循环 ---- */
    while (1) {
        /* 秒计数器 */
        uptime_cnt++;
        if (uptime_cnt >= 100) { uptime_cnt = 0; system_uptime_s++; }

        /* 按键 */
        uint8_t key = KEY_Scan(0);
        if (key == KEY1_PRESS) {
            current_page = (current_page + 1) % 3;
            Display_Refresh(1);
        } else if (key == KEY2_PRESS) {
            AI_Init(&my_detector, 0.2f, 100);
            current_ai_state = AI_STATE_LEARNING;
            system_mode      = 0;
            aht20_fail_cnt   = 0;
            bmp280_fail_cnt  = 0;
            test_aht20_init_status  = AHT20_Init();
            aht20_healthy           = (test_aht20_init_status  == 0);
            test_bmp280_init_status = BMP280_Init();
            bmp280_healthy          = (test_bmp280_init_status == 0);
            LED_Off(0);
            Display_Refresh(1);
        }

        /* 1.5 秒传感器轮询 */
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

            /* BMP280 */
            if (!bmp280_healthy) {
                if (BMP280_Init() == 0) { bmp280_healthy = 1; test_bmp280_init_status = 0; bmp280_fail_cnt = 0; }
            }
            if (bmp280_healthy) {
                float t, p;
                if (BMP280_ReadData(&t, &p) == 0) {
                    test_bmp280_temp = t; test_bmp280_press = p; bmp280_fail_cnt = 0;
                } else {
                    bmp280_fail_cnt++;
                    if (bmp280_fail_cnt >= 3) bmp280_healthy = 0;
                }
            }

            /* AI */
            if (aht20_healthy) {
                float filt;
                uint8_t ns = AI_Process(&my_detector, test_aht20_temp, &filt);
                test_filtered_temp = filt;

                float dev = filt - my_detector.baseline_temp;
                for (uint8_t i = 0; i < 23; i++) dev_history[i] = dev_history[i+1];
                dev_history[23] = dev;

                if (current_ai_state != AI_STATE_ANOMALY && ns == AI_STATE_ANOMALY)
                    Log_Add(&my_log_buffer, system_uptime_s,
                            my_detector.baseline_temp, filt, test_bmp280_press);
                current_ai_state = ns;
            }

            system_mode = (current_ai_state == AI_STATE_ANOMALY ||
                           !aht20_healthy || !bmp280_healthy);
            ui_dirty = 1;
        }

        /* UI 刷新 */
        if (ui_dirty) { ui_dirty = 0; Display_Refresh(0); }

        /* Beacon */
        Draw_Beacon();

        /* LED 驱动 */
        if (system_mode == 0) {
            LED_Off(0);
            LED_ProcessBreathing();
        } else {
            LED_Off(1);
            flash_cnt++;
            if (flash_cnt >= 10) { flash_cnt = 0; LED_Toggle(0); }
        }

        Delay(72000);   /* ~10ms */
    }
}

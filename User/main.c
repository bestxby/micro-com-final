#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "bmp280.h"
#include "lcd.h"
#include "ai_detect.h"
#include "anomaly_log.h"
#include "game.h"
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
volatile uint8_t  current_page    = 0;    /* 0=实时监测, 1=AI自适应, 2=历史日志, 3=飞鸟游戏 */
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
    return DARK_GRAY;  /* 正常: 板岩灰/深蓝 */
}

/* ---- 绘制顶部状态栏 ---- */
static void Draw_TopBar(void) {
    uint16_t bg = TopBarBg();
    LCD_FillRect(0, 0, LCD_WIDTH, TOP_BAR_H, bg);

    char buf[40];
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
    LCD_ShowString(x, 4, buf, fg, bg);
}

/* ---- 绘制副标题 & 分隔线 ---- */
static void Draw_Header(const char *title) {
    LCD_DrawLine(0, SEP_Y1, LCD_WIDTH - 1, SEP_Y1, GRAY);   /* 上分隔线 */
    LCD_FillRect(0, SEP_Y1 + 1, LCD_WIDTH, SEP_Y2 - SEP_Y1 - 1, BLACK);
    LCD_ShowString(12, TITLE_Y, title, CYAN, BLACK);
    LCD_DrawLine(0, SEP_Y2, LCD_WIDTH - 1, SEP_Y2, GRAY);   /* 下分隔线 */
}

/* ---- 绘制底部翻页指示点 ---- */
static void Draw_PageDots(void) {
    uint16_t dots[4] = {204, 224, 244, 264};
    for (uint8_t i = 0; i < 4; i++) {
        if (i == current_page) {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 3, CYAN);
        } else {
            LCD_FillCircle(dots[i], PAGE_DOT_Y, 3, BLACK);
            LCD_DrawCircle(dots[i], PAGE_DOT_Y, 3, GRAY);
        }
    }
}

/* ---- 底部系统信息 ---- */
static void Draw_BottomInfo(void) {
    char buf[24];
    sprintf(buf, "UPTIME: %5d s", system_uptime_s);
    LCD_FillRect(0, BOTTOM_Y, 150, 16, BLACK);
    LCD_ShowString(16, BOTTOM_Y, buf, GRAY, BLACK);

    sprintf(buf, "GROUP 05");
    uint16_t x = LCD_WIDTH - (uint16_t)strlen(buf) * 8 - 16;
    LCD_FillRect(x, BOTTOM_Y, 80, 16, BLACK);
    LCD_ShowString(x, BOTTOM_Y, buf, GRAY, BLACK);
}

/* ---- Page 1: 偏差折线图 (24 个采样点, 完美契合右侧卡片) ---- */
static void Draw_DevChart(void) {
    uint16_t cx = 216, cy = 56, cw = 248, ch = 220;
    
    // 零线 (Y=166)
    uint16_t zero_y = cy + ch / 2;
    
    /* 绘制微弱的网格线，实现示波器效果 */
    // 1. 水平辅助网格线
    LCD_DrawLine(cx + 6, zero_y - 45, cx + cw - 7, zero_y - 45, DARK_GRAY);  /* +3.0C 辅助线 */
    LCD_DrawLine(cx + 6, zero_y + 45, cx + cw - 7, zero_y + 45, DARK_GRAY);  /* -3.0C 辅助线 */
    
    // 2. 垂直辅助网格线
    for (uint16_t gx = cx + 30; gx < cx + cw - 10; gx += 38) {
        LCD_DrawLine(gx, cy + 10, gx, cy + ch - 10, DARK_GRAY);
    }
    
    LCD_DrawLine(cx + 6, zero_y, cx + cw - 7, zero_y, GRAY);       /* 零偏差基准线 */
    LCD_DrawLine(cx + 6, cy + 10, cx + 6, cy + ch - 10, GRAY);    /* 左侧轴 */
    LCD_DrawLine(cx + cw - 6, cy + 10, cx + cw - 6, cy + ch - 10, GRAY);  /* 右侧轴 */

    /* 折线绘制 */
    float scale = 15.0f;   /* 比例: 1°C = 15 像素 */
    for (uint8_t i = 0; i < 23; i++) {
        uint16_t x1 = cx + 12 + i * 10;
        uint16_t x2 = cx + 12 + (i + 1) * 10;
        float y1_val = (float)zero_y - dev_history[i]   * scale;
        float y2_val = (float)zero_y - dev_history[i+1] * scale;

        // 绘图界限裁剪保护
        if (y1_val < cy + 10) y1_val = cy + 10;
        if (y1_val > cy + ch - 10) y1_val = cy + ch - 10;
        if (y2_val < cy + 10) y2_val = cy + 10;
        if (y2_val > cy + ch - 10) y2_val = cy + ch - 10;

        LCD_DrawLine(x1, (uint16_t)y1_val, x2, (uint16_t)y2_val, YELLOW);
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
        else
            Draw_Header("[4] FLAPPY BIRD MINI-GAME");
    }

    uint16_t y;

    /* ================================================================
     * PAGE 0: 实时监测 (卡片式栅格布局: X左侧[16..232] 右侧[248..464])
     * ================================================================ */
    if (current_page == 0) {
        
        // ------------------ 左侧栏 ------------------
        
        // Card 1: Temperature Raw
        if (force_refresh) {
            LCD_FillRect(16, 56, 216, 68, DARK_GRAY);
            LCD_DrawRect(16, 56, 216, 68, GRAY);
            LCD_ShowString(28, 62, "Temp Raw:", CYAN, DARK_GRAY);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", test_aht20_temp);
            LCD_ShowString(28, 80, buf, WHITE, DARK_GRAY);
            
            // 简单进度条
            float t = test_aht20_temp;
            if (t < 10.0f) t = 10.0f;
            if (t > 40.0f) t = 40.0f;
            uint16_t w = (uint16_t)((t - 10.0f) * 6.0f); // 10..40度映射到0..180px
            LCD_FillRect(28, 102, 180, 8, BLACK);
            LCD_FillRect(28, 102, w, 8, (t > 30.0f || t < 15.0f) ? RED : GREEN);
        } else {
            LCD_ShowString(28, 80, "[ERROR]", RED, DARK_GRAY);
        }

        // Card 2: Humidity
        if (force_refresh) {
            LCD_FillRect(16, 132, 216, 68, DARK_GRAY);
            LCD_DrawRect(16, 132, 216, 68, GRAY);
            LCD_ShowString(28, 138, "Humidity:", CYAN, DARK_GRAY);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f %%", test_aht20_humi);
            LCD_ShowString(28, 156, buf, WHITE, DARK_GRAY);
            
            float h = test_aht20_humi;
            if (h < 0.0f) h = 0.0f;
            if (h > 100.0f) h = 100.0f;
            uint16_t w = (uint16_t)(h * 1.8f); // 0..100% 映射到0..180px
            LCD_FillRect(28, 178, 180, 8, BLACK);
            LCD_FillRect(28, 178, w, 8, BLUE);
        } else {
            LCD_ShowString(28, 156, "[ERROR]", RED, DARK_GRAY);
        }

        // Card 3: Pressure
        if (force_refresh) {
            LCD_FillRect(16, 208, 216, 68, DARK_GRAY);
            LCD_DrawRect(16, 208, 216, 68, GRAY);
            LCD_ShowString(28, 214, "Pressure:", CYAN, DARK_GRAY);
        }
        if (bmp280_healthy) {
            sprintf(buf, "%6.0f Pa", test_bmp280_press);
            LCD_ShowString(28, 238, buf, WHITE, DARK_GRAY);
        } else {
            LCD_ShowString(28, 238, "[ERROR]", RED, DARK_GRAY);
        }

        // ------------------ 右侧栏 ------------------
        
        // Card 4: AI Filtered Temp
        if (force_refresh) {
            LCD_FillRect(248, 56, 216, 68, DARK_GRAY);
            LCD_DrawRect(248, 56, 216, 68, GRAY);
            LCD_ShowString(260, 62, "AI Filtered Temp:", CYAN, DARK_GRAY);
        }
        if (aht20_healthy) {
            sprintf(buf, "%5.2f C", test_filtered_temp);
            LCD_ShowString(260, 80, buf, YELLOW, DARK_GRAY);
            LCD_ShowString(260, 100, "(EMA alpha=0.2)", GRAY, DARK_GRAY);
        } else {
            LCD_ShowString(260, 80, "[ERROR]", RED, DARK_GRAY);
        }

        // Card 5: System Status & Health Info
        if (force_refresh) {
            LCD_FillRect(248, 132, 216, 144, DARK_GRAY);
            LCD_DrawRect(248, 132, 216, 144, GRAY);
            LCD_ShowString(260, 138, "System Health:", CYAN, DARK_GRAY);
            LCD_ShowString(260, 156, "AHT20  :", GRAY, DARK_GRAY);
            LCD_ShowString(260, 174, "BMP280 :", GRAY, DARK_GRAY);
            
            LCD_ShowString(260, 198, "AI State:", CYAN, DARK_GRAY);
        }
        
        LCD_ShowString(324, 156, aht20_healthy ? "ONLINE " : "OFFLINE", aht20_healthy ? GREEN : RED, DARK_GRAY);
        LCD_ShowString(324, 174, bmp280_healthy ? "ONLINE " : "OFFLINE", bmp280_healthy ? GREEN : RED, DARK_GRAY);
        
        if (!aht20_healthy || !bmp280_healthy) {
            LCD_ShowString(260, 216, "FAULT/SENSOR ERR", RED, DARK_GRAY);
        } else if (current_ai_state == AI_STATE_LEARNING) {
            uint8_t pct = (my_detector.learning_samples * 100) / 100;
            sprintf(buf, "LEARNING (%3d%%)", pct);
            LCD_ShowString(260, 216, buf, BLUE, DARK_GRAY);
        } else if (current_ai_state == AI_STATE_NORMAL) {
            LCD_ShowString(260, 216, "MONITOR NORMAL  ", GREEN, DARK_GRAY);
        } else {
            LCD_ShowString(260, 216, "ANOMALY ALARM!! ", RED, DARK_GRAY);
        }
        
        sprintf(buf, "Board ID: GROUP 05");
        LCD_ShowString(260, 246, buf, GRAY, DARK_GRAY);
    }

    /* ================================================================
     * PAGE 1: AI自适应基准学习 (左侧栏[16..200] 右侧趋势图[216..464])
     * ================================================================ */
    else if (current_page == 1) {
        
        // ------------------ 左侧诊断卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(16, 56, 184, 220, DARK_GRAY);
            LCD_DrawRect(16, 56, 184, 220, GRAY);
            
            LCD_ShowString(24, 62, "Baseline Temp:", CYAN, DARK_GRAY);
            LCD_ShowString(24, 96, "Filtered Temp:", CYAN, DARK_GRAY);
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
            LCD_DrawRect(24, 244, 152, 12, GRAY);
            uint32_t cur = my_detector.learning_samples;
            uint32_t max = my_detector.max_learning_samples;
            if (cur > max) cur = max;
            uint16_t pw = (uint16_t)(cur * 148 / max);
            uint16_t pc = my_detector.is_learning_done ? GREEN : BLUE;
            if (pw > 0) LCD_FillRect(26, 246, pw, 8, pc);
            if (pw < 148) LCD_FillRect(26 + pw, 246, 148 - pw, 8, BLACK);
        }

        // ------------------ 右侧趋势图卡片 ------------------
        if (force_refresh) {
            LCD_FillRect(216, 56, 248, 220, DARK_GRAY);
            LCD_DrawRect(216, 56, 248, 220, GRAY);
            LCD_ShowString(228, 62, "Deviation Trend (+/-5 C)", CYAN, DARK_GRAY);
        }
        Draw_DevChart();
    }

    /* ================================================================
     * PAGE 2: 历史日志 (横屏优化：高画质时间轴，限制绘制最新4条防溢出)
     * ================================================================ */
    else if (current_page == 2) {
        if (my_log_buffer.count == 0) {
            if (force_refresh) {
                LCD_ShowString(112, 150, "NO ANOMALY LOGS RECORDED", GRAY, BLACK);
            }
        } else {
            /* 1. 先绘制时间轴垂直轴线 */
            if (force_refresh) {
                LCD_DrawLine(24, 56, 24, 276, GRAY);
            }
            
            /* 2. 遍历环形缓冲区绘制历史异常记录 */
            uint8_t show_count = my_log_buffer.count > 4 ? 4 : my_log_buffer.count;
            uint8_t start_idx = (my_log_buffer.head + MAX_ANOMALY_LOGS - my_log_buffer.count) % MAX_ANOMALY_LOGS;
            
            for (uint8_t i = 0; i < show_count; i++) {
                uint8_t idx = (start_idx + (my_log_buffer.count - show_count) + i) % MAX_ANOMALY_LOGS;
                AnomalyEvent *ev = &my_log_buffer.logs[idx];
                uint16_t row_y = 56 + i * 54; /* 4个卡片，高度48，间距6 */

                if (force_refresh) {
                    /* 时间轴节点 */
                    LCD_DrawLine(24, row_y + 24, 42, row_y + 24, GRAY);
                    LCD_FillCircle(24, row_y + 24, 3, RED);

                    /* 右侧日志卡片背景与细灰色边框 */
                    LCD_FillRect(42, row_y, 422, 48, DARK_GRAY);
                    LCD_DrawRect(42, row_y, 422, 48, GRAY);

                    /* 卡片内部信息 */
                    sprintf(buf, "#%d [UPTIME: %5d s]", i + 1, ev->timestamp_s);
                    LCD_ShowString(52, row_y + 6, buf, YELLOW, DARK_GRAY);

                    sprintf(buf, "Baseline: %5.2f C -> Current: %5.2f C", ev->baseline_temp, ev->current_temp);
                    LCD_ShowString(52, row_y + 26, buf, RED, DARK_GRAY);

                    sprintf(buf, "Press: %6.0f Pa", ev->current_press);
                    LCD_ShowString(280, row_y + 6, buf, GRAY, DARK_GRAY);
                }
            }
        }
    }
    
    /* ================================================================
     * PAGE 3: Flappy Bird 像素飞鸟小游戏
     * ================================================================ */
    else {
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
        uint16_t fg = (bg == RED || bg == BLUE) ? WHITE : GREEN;
        LCD_ShowString(LCD_WIDTH - 16, 4, s, fg, bg);
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

        if (key == KEY1_PRESS) {
            /* KEY1 按键: 循环切换页面 (0 -> 1 -> 2 -> 3 -> 0) */
            current_page = (current_page + 1) % 4;
            if (current_page == 3) {
                Game_Init(); /* 切到游戏页面时初始化游戏数据 */
            }
            Display_Refresh(1);
        } else if (key == KEY2_PRESS) {
            if (current_page != 3) {
                /* KEY2 按键 (非游戏模式): 硬件自愈重置 */
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
                            my_detector.baseline_temp, filt, test_bmp280_press);
                current_ai_state = ns;
            }

            system_mode = (current_ai_state == AI_STATE_ANOMALY ||
                           !aht20_healthy || !bmp280_healthy);
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

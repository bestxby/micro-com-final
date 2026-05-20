#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "bmp280.h"
#include "lcd.h"
#include "ai_detect.h"
#include "anomaly_log.h"
#include <stdio.h>

/* 全局调试测试变量 */
volatile float test_aht20_temp = 0.0f;
volatile float test_aht20_humi = 0.0f;
volatile float test_bmp280_temp = 0.0f;
volatile float test_bmp280_press = 0.0f;

volatile uint8_t test_aht20_init_status = 1;  /* 0 = 成功, 1 = 失败 */
volatile uint8_t test_bmp280_init_status = 1; /* 0 = 成功, 1 = 失败 */

/* 传感器运行在线健康状态 (0 = 异常/掉线, 1 = 正常) */
volatile uint8_t aht20_healthy = 1;
volatile uint8_t bmp280_healthy = 1;

/* 连续采样失败计数器 */
uint8_t aht20_fail_cnt = 0;
uint8_t bmp280_fail_cnt = 0;

/* AI 检测器与异常日志环形缓冲区结构体变量 */
AI_Detector my_detector;
AnomalyLogBuffer my_log_buffer;
volatile float test_filtered_temp = 0.0f;
volatile uint8_t current_ai_state = AI_STATE_LEARNING;

/* 用于 Page 1 绘制实时波动曲线的历史偏离量缓冲区 (共 20 个采样点) */
float dev_history[20] = {0.0f};

/* 系统上电运行时间计数器 (单位: 秒) */
volatile uint32_t system_uptime_s = 0;

/* 系统 UI 运行状态模式 (对应 LED 指示): 0 = 正常/自适应学习期, 1 = 异常报警 */
volatile uint8_t system_mode = 0;
volatile uint8_t current_page = 0;   /* 0 = 实时监测页面, 1 = AI 学习状态页面, 2 = 历史报警日志页面 */
volatile uint8_t last_page = 99;     /* 用于跟踪页面切换以触发全屏清除，避免文字残留 */

/**
  * @brief  简易软件延时函数。
  * @param  nCount: 延时循环计数值。
  * @retval 无
  */
void Delay(__IO uint32_t nCount)
{
    for (; nCount != 0; nCount--);
}

/**
  * @brief  在 Page 1 绘制实时温度偏离值波动折线图。
  * @retval 无
  */
void Draw_Deviation_Chart(void)
{
    /* 清除折线图绘制区域的旧图形 */
    LCD_DrawRectangle_Filled(21, 201, 219, 284, BLACK);
    
    /* 绘制网格与坐标轴 */
    LCD_DrawLine(20, 242, 220, 242, DARK_GRAY);  /* 零偏差基准线 */
    LCD_DrawLine(20, 200, 20, 285, DARK_GRAY);   /* Y 轴边界 */
    LCD_DrawLine(220, 200, 220, 285, DARK_GRAY);  /* 右侧边界 */

    /* 遍历绘制偏差走势折线 */
    for (uint8_t i = 0; i < 19; i++) {
        uint16_t x1 = 20 + i * 10;
        /* 计算 Y 轴像素坐标 (中心 Y=242. 比例: 1度偏差 = 8.5像素) */
        float y1_val = 242.0f - (dev_history[i] * 8.5f);
        float y2_val = 242.0f - (dev_history[i+1] * 8.5f);

        /* 坐标上下界限限幅，防止画出界 */
        if (y1_val < 201.0f) y1_val = 201.0f;
        if (y1_val > 284.0f) y1_val = 284.0f;
        if (y2_val < 201.0f) y2_val = 201.0f;
        if (y2_val > 284.0f) y2_val = 284.0f;

        uint16_t y1 = (uint16_t)y1_val;
        uint16_t y2 = (uint16_t)y2_val;

        LCD_DrawLine(x1, y1, x1 + 10, y2, YELLOW);
    }
}

/**
  * @brief  绘制屏幕底部桌面翻页指示小圆点 (Page Dots)。
  * @retval 无
  */
void Draw_Page_Dots(void)
{
    uint16_t dot_xs[3] = {104, 120, 136};
    uint16_t y = 305;
    
    for (uint8_t i = 0; i < 3; i++) {
        if (i == current_page) {
            /* 当前页高亮绘制为实心蓝绿色圆点 */
            LCD_DrawCircle_Filled(dot_xs[i], y, 3, CYAN);
        } else {
            /* 非当前页擦除并绘制空心灰色圆圈 */
            LCD_DrawCircle_Filled(dot_xs[i], y, 3, BLACK);
            LCD_DrawCircle(dot_xs[i], y, 3, GRAY);
        }
    }
}

/**
  * @brief  根据当前选择的页面，刷新绘制 LCD 屏幕内容。
  * @param  force_refresh: 设为 1 时强制完整重绘静态文字背景 (如切页时)
  * @retval 无
  */
void Display_Refresh(uint8_t force_refresh)
{
    char text_buf[32];

    /* 1. 检测到切页时，执行全屏擦除，防止静态标签文字残留重叠 */
    if (current_page != last_page || force_refresh) {
        LCD_Clear(BLACK);
        last_page = current_page;
        force_refresh = 1; /* 强制重绘静态标签 */
    }

    /* 2. 绘制顶部系统运行状态警示条 */
    if (aht20_healthy == 0 || bmp280_healthy == 0) {
        LCD_DrawRectangle_Filled(0, 0, 239, 24, RED);
        LCD_ShowString(36, 4, "! SENSOR DISCONNECTED !", WHITE, RED);
    } else if (current_ai_state == AI_STATE_ANOMALY) {
        LCD_DrawRectangle_Filled(0, 0, 239, 24, RED);
        LCD_ShowString(40, 4, "! ANOMALY DETECTED !", WHITE, RED);
    } else if (current_ai_state == AI_STATE_LEARNING) {
        LCD_DrawRectangle_Filled(0, 0, 239, 24, BLUE);
        LCD_ShowString(36, 4, "* LEARNING BASELINE *", WHITE, BLUE);
    } else {
        LCD_DrawRectangle_Filled(0, 0, 239, 24, DARK_GRAY);
        LCD_ShowString(52, 4, "* SYSTEM NORMAL *", GREEN, DARK_GRAY);
    }

    /* 3. 绘制副标题 */
    if (force_refresh) {
        LCD_DrawLine(0, 48, 239, 48, GRAY);
        if (current_page == 0) {
            LCD_ShowString(12, 30, "[1] REAL-TIME MONITOR", CYAN, BLACK);
        } else if (current_page == 1) {
            LCD_ShowString(12, 30, "[2] AI BASELINE LEARN", CYAN, BLACK);
        } else {
            LCD_ShowString(12, 30, "[3] HISTORY LOG ARCHIVE", CYAN, BLACK);
        }
    }

    /* 4. 绘制不同页面具体的数据面板 */
    if (current_page == 0) {
        /* PAGE 0: 实时监控数据 */
        if (force_refresh) {
            LCD_ShowString(16, 65,  "Temp Raw:            C", WHITE, BLACK);
            LCD_ShowString(16, 95,  "Humidity:            %", WHITE, BLACK);
            LCD_ShowString(16, 125, "Pressure:            Pa", WHITE, BLACK);
            LCD_ShowString(16, 155, "Sys Status: ", WHITE, BLACK);
            LCD_ShowString(16, 185, "Board ID: GROUP 05", WHITE, BLACK);
            LCD_ShowString(16, 215, "Uptime  :            s", WHITE, BLACK);
        }

        /* 原位使用固定宽度格式化字符串重写变量，防止闪烁 */
        if (aht20_healthy) {
            sprintf(text_buf, "%5.2f", test_aht20_temp);
            LCD_ShowString(96, 65, text_buf, GREEN, BLACK);
            sprintf(text_buf, "%5.2f", test_aht20_humi);
            LCD_ShowString(96, 95, text_buf, GREEN, BLACK);
        } else {
            LCD_ShowString(96, 65, "[ERROR]", RED, BLACK);
            LCD_ShowString(96, 95, "[ERROR]", RED, BLACK);
        }

        if (bmp280_healthy) {
            sprintf(text_buf, "%6.0f", test_bmp280_press);
            LCD_ShowString(96, 125, text_buf, GREEN, BLACK);
        } else {
            LCD_ShowString(96, 125, " [ERROR]", RED, BLACK);
        }

        if (aht20_healthy == 0 || bmp280_healthy == 0) {
            LCD_ShowString(112, 155, "SENSOR ERROR   ", RED, BLACK);
        } else if (current_ai_state == AI_STATE_LEARNING) {
            uint8_t pct = (my_detector.learning_samples * 100) / 100;
            sprintf(text_buf, "LEARNING (%3d%%)", pct);
            LCD_ShowString(112, 155, text_buf, BLUE, BLACK);
        } else if (current_ai_state == AI_STATE_NORMAL) {
            LCD_ShowString(112, 155, "NORMAL         ", GREEN, BLACK);
        } else {
            LCD_ShowString(112, 155, "ANOMALY DETECT ", RED, BLACK);
        }

        sprintf(text_buf, "%5d", system_uptime_s);
        LCD_ShowString(96, 215, text_buf, YELLOW, BLACK);
    } 
    else if (current_page == 1) {
        /* PAGE 1: AI自适应基准学习状态与实时偏差图 */
        if (force_refresh) {
            LCD_ShowString(16, 65,  "Base Temp:           C", WHITE, BLACK);
            LCD_ShowString(16, 87,  "Filt Temp:           C", WHITE, BLACK);
            LCD_ShowString(16, 109, "Deviation:           C", WHITE, BLACK);
            
            /* 绘制偏差对称指示彩条背景刻度线 */
            LCD_DrawRectangle(29, 127, 211, 135, DARK_GRAY);
            LCD_DrawLine(120, 125, 120, 137, GRAY); /* 0偏差中心线 */

            LCD_ShowString(16, 145, "Samples  :     / 100", WHITE, BLACK);
            
            /* 绘制学习进度水平进度条背景框 */
            LCD_DrawRectangle(29, 164, 211, 172, DARK_GRAY);

            LCD_ShowString(16, 185, "Real-time Deviation Chart (+/-5 C)", GRAY, BLACK);
            LCD_DrawRectangle(20, 200, 220, 285, GRAY);
        }

        if (aht20_healthy) {
            sprintf(text_buf, "%5.2f", my_detector.baseline_temp);
            LCD_ShowString(96, 65, text_buf, GREEN, BLACK);

            sprintf(text_buf, "%5.2f", test_filtered_temp);
            LCD_ShowString(96, 87, text_buf, GREEN, BLACK);

            float dev = test_filtered_temp - my_detector.baseline_temp;
            sprintf(text_buf, "%+5.2f", dev);
            uint16_t dev_color = GREEN;
            if (dev > 3.0f || dev < -3.0f) dev_color = RED;
            else if (dev > 1.5f || dev < -1.5f) dev_color = YELLOW;
            LCD_ShowString(96, 109, text_buf, dev_color, BLACK);

            /* 绘制双向对称偏差动态指示柱 (X在30到210像素) */
            float dev_lim = dev;
            if (dev_lim > 5.0f) dev_lim = 5.0f;
            if (dev_lim < -5.0f) dev_lim = -5.0f;
            int16_t w = (int16_t)(dev_lim * 18.0f); /* -5C到+5C对应30px到210px，0度在120px */
            
            if (w > 0) {
                /* 清理左侧负偏差图形残留 */
                LCD_DrawRectangle_Filled(30, 128, 119, 134, BLACK);
                /* 绘制正偏差填充条 */
                LCD_DrawRectangle_Filled(120, 128, 120 + w, 134, dev_color);
                /* 清理右侧剩余背景区域 */
                if (120 + w < 210) {
                    LCD_DrawRectangle_Filled(121 + w, 128, 210, 134, BLACK);
                }
            } else if (w < 0) {
                /* 清理左侧剩余背景区域 */
                if (120 + w > 30) {
                    LCD_DrawRectangle_Filled(30, 128, 120 + w - 1, 134, BLACK);
                }
                /* 绘制负偏差填充条 */
                LCD_DrawRectangle_Filled(120 + w, 128, 120, 134, dev_color);
                /* 清理右侧正偏差图形残留 */
                LCD_DrawRectangle_Filled(121, 128, 210, 134, BLACK);
            } else {
                /* 偏差完全为0，擦除整条 */
                LCD_DrawRectangle_Filled(30, 128, 210, 134, BLACK);
            }
            /* 重新画一下中间被覆盖的0基准中轴线 */
            LCD_DrawLine(120, 125, 120, 137, GRAY);
        } else {
            LCD_ShowString(96, 65, "[ERROR]", RED, BLACK);
            LCD_ShowString(96, 87, "[ERROR]", RED, BLACK);
            LCD_ShowString(96, 109, "[ERROR]", RED, BLACK);
            /* 清空整条区域 */
            LCD_DrawRectangle_Filled(30, 128, 210, 134, BLACK);
            LCD_DrawLine(120, 125, 120, 137, GRAY);
        }

        sprintf(text_buf, "%3d", my_detector.learning_samples);
        LCD_ShowString(96, 145, text_buf, YELLOW, BLACK);

        /* 绘制基准自适应学习阶段进度条 */
        uint32_t cur_samples = my_detector.learning_samples;
        uint32_t max_samples = my_detector.max_learning_samples;
        if (cur_samples > max_samples) cur_samples = max_samples;
        
        /* 进度条宽度为 180 像素，范围在 X=30 到 X=210 */
        uint16_t bar_w = (uint16_t)(cur_samples * 180 / max_samples);
        uint16_t progress_color = BLUE;
        if (my_detector.is_learning_done) {
            progress_color = GREEN; /* 学习完成转为绿色 */
        }

        if (bar_w > 0) {
            LCD_DrawRectangle_Filled(30, 165, 30 + bar_w - 1, 171, progress_color);
        }
        if (30 + bar_w < 210) {
            LCD_DrawRectangle_Filled(30 + bar_w, 165, 210, 171, BLACK);
        }

        /* 刷新折线图 */
        Draw_Deviation_Chart();
    } 
    else {
        /* PAGE 2: 异常历史记录列表 */
        if (my_log_buffer.count == 0) {
            LCD_ShowString(24, 120, "NO ANOMALY LOGS RECORDED", GRAY, BLACK);
        } else {
            /* 遍历环形缓冲区绘制历史异常记录 */
            uint8_t start_idx = (my_log_buffer.head + MAX_ANOMALY_LOGS - my_log_buffer.count) % MAX_ANOMALY_LOGS;
            for (uint8_t i = 0; i < my_log_buffer.count; i++) {
                uint8_t idx = (start_idx + i) % MAX_ANOMALY_LOGS;
                AnomalyEvent *ev = &my_log_buffer.logs[idx];
                uint16_t row_y = 60 + i * 44;

                if (force_refresh) {
                    sprintf(text_buf, "%d. UPTIME: %4d s", i + 1, ev->timestamp_s);
                    LCD_ShowString(12, row_y, text_buf, YELLOW, BLACK);

                    sprintf(text_buf, "   BASE:%4.1f C  CUR:%4.1f C", ev->baseline_temp, ev->current_temp);
                    LCD_ShowString(12, row_y + 18, text_buf, RED, BLACK);
                    
                    if (i < my_log_buffer.count - 1) {
                        LCD_DrawLine(12, row_y + 38, 228, row_y + 38, DARK_GRAY);
                    }
                }
            }
        }
    }

    /* 5. 绘制底部的桌面式翻页指示小圆点 */
    Draw_Page_Dots();
}

/**
  * @brief  前台运行心跳旋转Beacon微动画 (每 200ms 刷新一次字符)。
  * @retval 无
  */
void Draw_Alive_Beacon(void)
{
    static uint8_t beacon_idx = 0;
    static uint32_t beacon_tick = 0;
    const char beacon_chars[4] = {'|', '/', '-', '\\'};
    
    beacon_tick++;
    if (beacon_tick >= 20) { /* 主循环10ms心跳，20次 = 200ms */
        beacon_tick = 0;
        char buf[2] = {beacon_chars[beacon_idx], '\0'};
        
        /* 根据当前系统警报状态自适应匹配背景色 */
        uint16_t bg_color = DARK_GRAY;
        uint16_t fg_color = GREEN;
        
        if (aht20_healthy == 0 || bmp280_healthy == 0) {
            bg_color = RED;
            fg_color = WHITE;
        } else if (current_ai_state == AI_STATE_ANOMALY) {
            bg_color = RED;
            fg_color = WHITE;
        } else if (current_ai_state == AI_STATE_LEARNING) {
            bg_color = BLUE;
            fg_color = WHITE;
        }
        
        LCD_ShowString(226, 4, buf, fg_color, bg_color);
        beacon_idx = (beacon_idx + 1) % 4;
    }
}

/**
  * @brief  主程序入口。
  * @param  无
  * @retval 整数值
  */
int main(void)
{
    uint32_t loop_counter = 0;
    uint32_t flash_counter = 0;
    uint32_t uptime_loop_counter = 0;
    uint8_t ui_refresh_tick = 0;

    /* 1. 初始化板载配置 of LED 与独立按键 */
    LED_Init();
    KEY_Init();

    /* 2. 初始化彩色 LCD 屏幕并执行黑屏清屏 */
    LCD_Init();
    LCD_Clear(BLACK);

    /* 3. 初始化板载 I2C 传感器并确立初始健康度 */
    test_aht20_init_status = AHT20_Init();
    aht20_healthy = (test_aht20_init_status == 0) ? 1 : 0;

    test_bmp280_init_status = BMP280_Init();
    bmp280_healthy = (test_bmp280_init_status == 0) ? 1 : 0;

    /* 4. 初始化 AI 检测器与异常数据历史缓冲区 */
    /* 配置平滑系数 Alpha = 0.2f, 学习采样期设定为 100 次 (约 150 秒) */
    AI_Init(&my_detector, 0.2f, 100);
    Log_Init(&my_log_buffer);

    /* 确保上电时外设输出端口状态一致 */
    LED_Off(0); /* 关闭 PC13 指示灯 */
    LED_Off(1); /* 关闭 PA0 的 TIM2 PWM 呼吸输出 */

    /* 启动时执行一次 UI 强制重绘 */
    Display_Refresh(1);

    /* 系统主循环 */
    while (1)
    {
        /* --- 10ms 系统心跳时序基础 --- */

        /* 5. 维护秒计数器 (基准循环每约 10ms 执行一次，100 次为 1 秒) */
        uptime_loop_counter++;
        if (uptime_loop_counter >= 100) {
            uptime_loop_counter = 0;
            system_uptime_s++;
        }

        /* 6. 按键扫描驱动 (消抖时间 10ms) */
        uint8_t key_pressed = KEY_Scan(0);
        if (key_pressed == KEY1_PRESS) {
            /* KEY1 按键: 切换屏幕显示页面 (0 -> 1 -> 2 -> 0) */
            current_page = (current_page + 1) % 3;
            Display_Refresh(1); /* 切页时触发即时清屏重绘 */
        } 
        else if (key_pressed == KEY2_PRESS) {
            /* KEY2 按键: 警报复位、计数清零与自适应算法参数重新初始化 */
            AI_Init(&my_detector, 0.2f, 100);
            current_ai_state = AI_STATE_LEARNING;
            system_mode = 0;
            aht20_fail_cnt = 0;
            bmp280_fail_cnt = 0;
            
            /* 强制重构物理硬件并尝试握手自愈 */
            test_aht20_init_status = AHT20_Init();
            aht20_healthy = (test_aht20_init_status == 0) ? 1 : 0;
            test_bmp280_init_status = BMP280_Init();
            bmp280_healthy = (test_bmp280_init_status == 0) ? 1 : 0;

            LED_Off(0); /* 强制关闭 PC13 闪烁 */
            Display_Refresh(1); /* 立即刷新界面状态 */
        }

        /* 7. 运行状态下自动掉线重连与周期轮询 (每隔 1.5 秒采样一次)
           150 次循环 * 10ms = 1500ms */
        loop_counter++;
        if (loop_counter >= 150) {
            loop_counter = 0;

            /* (A) AHT20 掉线自愈检测与数据读取 */
            if (aht20_healthy == 0) {
                if (AHT20_Init() == 0) {
                    aht20_healthy = 1;
                    test_aht20_init_status = 0;
                    aht20_fail_cnt = 0;
                }
            }
            if (aht20_healthy) {
                float temp = 0.0f;
                float humi = 0.0f;
                if (AHT20_ReadData(&temp, &humi) == 0) {
                    test_aht20_temp = temp;
                    test_aht20_humi = humi;
                    aht20_fail_cnt = 0;
                } else {
                    aht20_fail_cnt++;
                    if (aht20_fail_cnt >= 3) {
                        aht20_healthy = 0; /* 连续 3 次超时未应答判定掉线 */
                    }
                }
            }

            /* (B) BMP280 掉线自愈检测与数据读取 */
            if (bmp280_healthy == 0) {
                if (BMP280_Init() == 0) {
                    bmp280_healthy = 1;
                    test_bmp280_init_status = 0;
                    bmp280_fail_cnt = 0;
                }
            }
            if (bmp280_healthy) {
                float temp = 0.0f;
                float press = 0.0f;
                if (BMP280_ReadData(&temp, &press) == 0) {
                    test_bmp280_temp = temp;
                    test_bmp280_press = press;
                    bmp280_fail_cnt = 0;
                } else {
                    bmp280_fail_cnt++;
                    if (bmp280_fail_cnt >= 3) {
                        bmp280_healthy = 0; /* 连续 3 次超时未应答判定掉线 */
                    }
                }
            }

            /* (C) 若 AHT20 健康在线，送入 AI 核心引擎进行突变检测 */
            if (aht20_healthy && test_aht20_init_status == 0) {
                float filtered = 0.0f;
                uint8_t next_state = AI_Process(&my_detector, test_aht20_temp, &filtered);
                test_filtered_temp = filtered;

                /* 移位更新偏离值数组以供折线图绘制 */
                float current_dev = filtered - my_detector.baseline_temp;
                for (uint8_t i = 0; i < 19; i++) {
                    dev_history[i] = dev_history[i+1];
                }
                dev_history[19] = current_dev;

                /* 异常边缘触发检测：由 正常/学习 状态跃迁至 异常状态 时写入环形日志 */
                if (current_ai_state != AI_STATE_ANOMALY && next_state == AI_STATE_ANOMALY) {
                    Log_Add(&my_log_buffer, 
                            system_uptime_s, 
                            my_detector.baseline_temp, 
                            filtered, 
                            test_bmp280_press);
                }

                current_ai_state = next_state;
            }

            /* (D) 将 AI 检测出的状态和传感器物理在线状态映射到硬件 LED */
            if (current_ai_state == AI_STATE_ANOMALY || aht20_healthy == 0 || bmp280_healthy == 0) {
                system_mode = 1; /* 进入告警/硬件物理断开的异常模式 */
            } else {
                system_mode = 0; /* 正常模式 */
            }

            /* 标记传感器数据更新，用于刷新显示 */
            ui_refresh_tick = 1;
        }

        /* 8. 周期刷新动态数值 UI */
        if (ui_refresh_tick) {
            ui_refresh_tick = 0;
            Display_Refresh(0);
        }

        /* 9. 前台刷新运行指示 Beacon (心跳小助手) */
        Draw_Alive_Beacon();

        /* 10. 双 LED 灯物理状态机驱动调度 */
        if (system_mode == 0) {
            /* 正常模式：PA0 进行定时器 TIM2 PWM 呼吸渐变，PC13 常灭 */
            LED_Off(0);
            LED_ProcessBreathing();
        } else {
            /* 异常模式：PA0 强制灭，PC13 高频闪烁 (频率约 5Hz) */
            LED_Off(1);
            flash_counter++;
            if (flash_counter >= 10) {
                flash_counter = 0;
                LED_Toggle(0);
            }
        }

        /* 基础运行循环延时约 10ms (以 72MHz 运行时约需 72000 个指令周期) */
        Delay(72000);
    }
}

#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "bmp280.h"
#include "lcd.h"
#include "ai_detect.h"
#include "anomaly_log.h"
#include <stdio.h>

/* Global test variables for debugging */
volatile float test_aht20_temp = 0.0f;
volatile float test_aht20_humi = 0.0f;
volatile float test_bmp280_temp = 0.0f;
volatile float test_bmp280_press = 0.0f;

volatile uint8_t test_aht20_init_status = 1;  /* 0 = Success, 1 = Fail */
volatile uint8_t test_bmp280_init_status = 1; /* 0 = Success, 1 = Fail */

/* AI detector state variables */
AI_Detector my_detector;
AnomalyLogBuffer my_log_buffer;
volatile float test_filtered_temp = 0.0f;
volatile uint8_t current_ai_state = AI_STATE_LEARNING;

/* Historical deviations buffer for graphical chart (20 points) */
float dev_history[20] = {0.0f};

/* System uptime ticker in seconds */
volatile uint32_t system_uptime_s = 0;

/* System UI Mode (matches AI state for LED): 0 = Normal/Learning, 1 = Anomaly */
volatile uint8_t system_mode = 0;
volatile uint8_t current_page = 0;   /* 0 = Real-time, 1 = AI status, 2 = History logs */
volatile uint8_t last_page = 99;     /* Track page change to trigger clear screen */

/**
  * @brief  Simple software delay loop.
  * @param  nCount: Specifies the delay time.
  * @retval None
  */
void Delay(__IO uint32_t nCount)
{
    for (; nCount != 0; nCount--);
}

/**
  * @brief  Draws the real-time graphical deviation chart on Page 1.
  * @retval None
  */
void Draw_Deviation_Chart(void)
{
    /* Clear chart client area */
    LCD_DrawRectangle_Filled(21, 201, 219, 289, BLACK);
    
    /* Draw grid lines */
    LCD_DrawLine(20, 245, 220, 245, DARK_GRAY);  /* Zero baseline */
    LCD_DrawLine(20, 200, 20, 290, DARK_GRAY);   /* Y axis */
    LCD_DrawLine(220, 200, 220, 290, DARK_GRAY);  /* Border Right */

    /* Draw deviation plot lines */
    for (uint8_t i = 0; i < 19; i++) {
        uint16_t x1 = 20 + i * 10;
        /* Calculate Y coordinates (Middle Y=245. Scale: 1 C = 9 pixels, Max 5 C = 45 pixels) */
        float y1_val = 245.0f - (dev_history[i] * 9.0f);
        float y2_val = 245.0f - (dev_history[i+1] * 9.0f);

        /* Boundary clamp */
        if (y1_val < 200.0f) y1_val = 200.0f;
        if (y1_val > 290.0f) y1_val = 290.0f;
        if (y2_val < 200.0f) y2_val = 200.0f;
        if (y2_val > 290.0f) y2_val = 290.0f;

        uint16_t y1 = (uint16_t)y1_val;
        uint16_t y2 = (uint16_t)y2_val;

        LCD_DrawLine(x1, y1, x1 + 10, y2, YELLOW);
    }
}

/**
  * @brief  Updates the LCD display content based on the current page.
  * @param  force_refresh: Set to 1 to force full draw (e.g. on page switch)
  * @retval None
  */
void Display_Refresh(uint8_t force_refresh)
{
    char text_buf[32];

    /* 1. Page change screen clear */
    if (current_page != last_page || force_refresh) {
        LCD_Clear(BLACK);
        last_page = current_page;
        force_refresh = 1; /* Force redrawing static texts */
    }

    /* 2. Top Header Status Bar Rendering (Flicker-free header draw) */
    if (current_ai_state == AI_STATE_ANOMALY) {
        LCD_DrawRectangle_Filled(0, 0, 239, 24, RED);
        LCD_ShowString(40, 4, "! ANOMALY DETECTED !", WHITE, RED);
    } else if (current_ai_state == AI_STATE_LEARNING) {
        LCD_DrawRectangle_Filled(0, 0, 239, 24, BLUE);
        LCD_ShowString(36, 4, "* LEARNING BASELINE *", WHITE, BLUE);
    } else {
        LCD_DrawRectangle_Filled(0, 0, 239, 24, DARK_GRAY);
        LCD_ShowString(52, 4, "* SYSTEM NORMAL *", GREEN, DARK_GRAY);
    }

    /* 3. Subheader Rendering */
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

    /* 4. Page Content Rendering */
    if (current_page == 0) {
        /* PAGE 0: Real-Time Values */
        if (force_refresh) {
            LCD_ShowString(16, 65,  "Temp Raw:            C", WHITE, BLACK);
            LCD_ShowString(16, 95,  "Humidity:            %", WHITE, BLACK);
            LCD_ShowString(16, 125, "Pressure:            Pa", WHITE, BLACK);
            LCD_ShowString(16, 155, "Sys Status: ", WHITE, BLACK);
            LCD_ShowString(16, 185, "Board ID: GROUP 05", WHITE, BLACK);
            LCD_ShowString(16, 215, "Uptime  :            s", WHITE, BLACK);
        }

        /* Update dynamic variables in-place with fixed width formats */
        sprintf(text_buf, "%5.2f", test_aht20_temp);
        LCD_ShowString(96, 65, text_buf, GREEN, BLACK);

        sprintf(text_buf, "%5.2f", test_aht20_humi);
        LCD_ShowString(96, 95, text_buf, GREEN, BLACK);

        sprintf(text_buf, "%6.0f", test_bmp280_press);
        LCD_ShowString(96, 125, text_buf, GREEN, BLACK);

        if (current_ai_state == AI_STATE_LEARNING) {
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
        /* PAGE 1: AI Baseline Learning info & Line chart */
        if (force_refresh) {
            LCD_ShowString(16, 65,  "Base Temp:           C", WHITE, BLACK);
            LCD_ShowString(16, 95,  "Filt Temp:           C", WHITE, BLACK);
            LCD_ShowString(16, 125, "Deviation:           C", WHITE, BLACK);
            LCD_ShowString(16, 155, "Samples  :     / 100", WHITE, BLACK);
            
            /* Draw graph frame labels */
            LCD_ShowString(16, 182, "Real-time Deviation Chart (+/-5 C)", GRAY, BLACK);
            LCD_DrawRectangle(20, 200, 220, 290, GRAY);
        }

        sprintf(text_buf, "%5.2f", my_detector.baseline_temp);
        LCD_ShowString(96, 65, text_buf, GREEN, BLACK);

        sprintf(text_buf, "%5.2f", test_filtered_temp);
        LCD_ShowString(96, 95, text_buf, GREEN, BLACK);

        float dev = test_filtered_temp - my_detector.baseline_temp;
        sprintf(text_buf, "%+5.2f", dev);
        LCD_ShowString(96, 125, text_buf, (dev > 5.0f || dev < -5.0f) ? RED : GREEN, BLACK);

        sprintf(text_buf, "%3d", my_detector.learning_samples);
        LCD_ShowString(96, 155, text_buf, YELLOW, BLACK);

        /* Refresh real-time graph */
        Draw_Deviation_Chart();
    } 
    else {
        /* PAGE 2: Anomaly History Logs */
        if (my_log_buffer.count == 0) {
            LCD_ShowString(24, 120, "NO ANOMALY LOGS RECORDED", GRAY, BLACK);
        } else {
            /* Draw logs list in-place */
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
}

/**
  * @brief  Main program entry point.
  * @param  None
  * @retval int
  */
int main(void)
{
    uint32_t loop_counter = 0;
    uint32_t flash_counter = 0;
    uint32_t uptime_loop_counter = 0;
    uint8_t ui_refresh_tick = 0;

    /* 1. Initialize configured LEDs & Keys */
    LED_Init();
    KEY_Init();

    /* 2. Initialize LCD & clear screen */
    LCD_Init();
    LCD_Clear(BLACK);

    /* 3. Initialize Sensors (PB6=SCL, PB7=SDA) */
    test_aht20_init_status = AHT20_Init();
    test_bmp280_init_status = BMP280_Init();

    /* 4. Initialize AI Detector & Anomaly Log Circular Buffer */
    /* Alpha = 0.2f, Learning period = 100 samples (approx. 150 seconds) */
    AI_Init(&my_detector, 0.2f, 100);
    Log_Init(&my_log_buffer);

    /* Ensure initial output state is consistent */
    LED_Off(0); /* Turn off PC13 */
    LED_Off(1); /* Turn off PA0 (TIM2 PWM = 0) */

    /* Force draw screen on startup */
    Display_Refresh(1);

    /* Main Loop */
    while (1)
    {
        /* --- 10ms System Heartbeat Timing --- */

        /* 5. Maintain Uptime Ticker
           Since base loop delay is ~10ms, 100 loops equals approx. 1 second */
        uptime_loop_counter++;
        if (uptime_loop_counter >= 100) {
            uptime_loop_counter = 0;
            system_uptime_s++;
        }

        /* 6. Scan Keys (Checked every 10ms) */
        uint8_t key_pressed = KEY_Scan(0);
        if (key_pressed == KEY1_PRESS) {
            /* KEY1: UI Page switching (0 -> 1 -> 2 -> 0) */
            current_page = (current_page + 1) % 3;
            Display_Refresh(1); /* Immediate refresh on page change */
        } 
        else if (key_pressed == KEY2_PRESS) {
            /* KEY2: RESET AI Learning baseline and clear current anomaly status */
            AI_Init(&my_detector, 0.2f, 100);
            current_ai_state = AI_STATE_LEARNING;
            system_mode = 0;
            LED_Off(0); /* Ensure PC13 is off */
            Display_Refresh(1); /* Redraw screen immediately */
        }

        /* 7. Periodic Sensor Sampling & AI Calculation (every 1.5 seconds)
           150 loops * 10ms = 1500ms */
        loop_counter++;
        if (loop_counter >= 150) {
            loop_counter = 0;

            /* Read AHT20 if initialized successfully */
            if (test_aht20_init_status == 0) {
                float temp = 0.0f;
                float humi = 0.0f;
                if (AHT20_ReadData(&temp, &humi) == 0) {
                    test_aht20_temp = temp;
                    test_aht20_humi = humi;
                }
            }

            /* Read BMP280 if initialized successfully */
            if (test_bmp280_init_status == 0) {
                float temp = 0.0f;
                float press = 0.0f;
                if (BMP280_ReadData(&temp, &press) == 0) {
                    test_bmp280_temp = temp;
                    test_bmp280_press = press;
                }
            }

            /* Run AI Anomaly Detection if AHT20 reading is valid */
            if (test_aht20_init_status == 0) {
                float filtered = 0.0f;
                uint8_t next_state = AI_Process(&my_detector, test_aht20_temp, &filtered);
                test_filtered_temp = filtered;

                /* Shift deviation history array for Page 1 line graph */
                float current_dev = filtered - my_detector.baseline_temp;
                for (uint8_t i = 0; i < 19; i++) {
                    dev_history[i] = dev_history[i+1];
                }
                dev_history[19] = current_dev;

                /* Log anomaly event on positive-edge state transition */
                if (current_ai_state != AI_STATE_ANOMALY && next_state == AI_STATE_ANOMALY) {
                    Log_Add(&my_log_buffer, 
                            system_uptime_s, 
                            my_detector.baseline_temp, 
                            filtered, 
                            test_bmp280_press);
                }

                current_ai_state = next_state;

                /* Map AI state to LED Output Mode */
                if (current_ai_state == AI_STATE_ANOMALY) {
                    system_mode = 1; /* Anomaly state */
                } else {
                    system_mode = 0; /* Normal or Learning state */
                }
            }

            /* Trigger UI Update after sensor data refresh */
            ui_refresh_tick = 1;
        }

        /* 8. Asynchronous UI Screen Redraw
           Refresh dynamic values every 1.5 seconds in synch with sensors */
        if (ui_refresh_tick) {
            ui_refresh_tick = 0;
            Display_Refresh(0);
        }

        /* 9. LED Output Driver Multiplexing */
        if (system_mode == 0) {
            /* Normal Mode: PA0 Breaths, PC13 Off */
            LED_Off(0);
            LED_ProcessBreathing();
        } else {
            /* Anomaly Mode: PA0 Off, PC13 Flashes high frequency (approx. 5Hz) */
            LED_Off(1);
            flash_counter++;
            if (flash_counter >= 10) {
                flash_counter = 0;
                LED_Toggle(0);
            }
        }

        /* Base loop delay of ~10ms (approx. 72000 cycles at 72MHz) */
        Delay(72000);
    }
}

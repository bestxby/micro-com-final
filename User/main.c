#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "bmp280.h"
#include "ai_detect.h"
#include "anomaly_log.h"

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

/* System uptime ticker in seconds */
volatile uint32_t system_uptime_s = 0;

/* System UI Mode (matches AI state for LED): 0 = Normal/Learning (Breathing PA0), 1 = Anomaly (Flashing PC13) */
volatile uint8_t system_mode = 0;
volatile uint8_t current_page = 0;  /* 0 = Real-time, 1 = AI status, 2 = History logs */

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
  * @brief  Main program entry point.
  * @param  None
  * @retval int
  */
int main(void)
{
    uint32_t loop_counter = 0;
    uint32_t flash_counter = 0;
    uint32_t uptime_loop_counter = 0;

    /* 1. Initialize configured LEDs & Keys */
    LED_Init();
    KEY_Init();

    /* 2. Initialize Sensors (PB6=SCL, PB7=SDA) */
    test_aht20_init_status = AHT20_Init();
    test_bmp280_init_status = BMP280_Init();

    /* 3. Initialize AI Detector & Anomaly Log Circular Buffer */
    /* Alpha = 0.2f, Learning period = 100 samples (approx. 150 seconds) */
    AI_Init(&my_detector, 0.2f, 100);
    Log_Init(&my_log_buffer);

    /* Ensure initial output state is consistent */
    LED_Off(0); /* Turn off PC13 */
    LED_Off(1); /* Turn off PA0 (TIM2 PWM = 0) */

    /* Main Loop */
    while (1)
    {
        /* --- 10ms System Heartbeat Timing --- */

        /* 4. Maintain Uptime Ticker
           Since base loop delay is ~10ms, 100 loops equals approx. 1 second */
        uptime_loop_counter++;
        if (uptime_loop_counter >= 100) {
            uptime_loop_counter = 0;
            system_uptime_s++;
        }

        /* 5. Scan Keys */
        uint8_t key_pressed = KEY_Scan(0);
        if (key_pressed == KEY1_PRESS) {
            /* KEY1: UI Page switching (0 -> 1 -> 2 -> 0) */
            current_page = (current_page + 1) % 3;
        } 
        else if (key_pressed == KEY2_PRESS) {
            /* KEY2: RESET AI Learning baseline and clear current anomaly status */
            AI_Init(&my_detector, 0.2f, 100);
            current_ai_state = AI_STATE_LEARNING;
            system_mode = 0;
            LED_Off(0); /* Ensure PC13 is off */
        }

        /* 6. Periodic Sensor Sampling (every 1.5 seconds)
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
        }

        /* 7. LED Output Driver Multiplexing */
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

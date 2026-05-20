#include "main.h"
#include "led.h"
#include "key.h"
#include "aht20.h"
#include "bmp280.h"

/* Global test variables for debugging */
volatile float test_aht20_temp = 0.0f;
volatile float test_aht20_humi = 0.0f;
volatile float test_bmp280_temp = 0.0f;
volatile float test_bmp280_press = 0.0f;

volatile uint8_t test_aht20_init_status = 1;  /* 0 = Success, 1 = Fail */
volatile uint8_t test_bmp280_init_status = 1; /* 0 = Success, 1 = Fail */

/* System UI / LED State: 0 = Normal (Breathing PA0), 1 = Anomaly (Flashing PC13) */
volatile uint8_t system_mode = 0;

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

    /* 1. Initialize configured LEDs & Keys */
    LED_Init();
    KEY_Init();

    /* 2. Initialize Sensors (PB6=SCL, PB7=SDA) */
    test_aht20_init_status = AHT20_Init();
    test_bmp280_init_status = BMP280_Init();

    /* Ensure initial output state is consistent */
    LED_Off(0); /* Turn off PC13 */
    LED_Off(1); /* Turn off PA0 (TIM2 PWM = 0) */

    /* Main Loop */
    while (1)
    {
        /* 3. Scan Keys (checked every ~10ms) */
        uint8_t key_pressed = KEY_Scan(0);
        if (key_pressed == KEY1_PRESS) {
            /* Toggle mode between Normal (0) and Anomaly (1) */
            system_mode = !system_mode;
            
            /* Clean up previous mode outputs */
            if (system_mode == 0) {
                LED_Off(0); /* Turn off PC13 flash */
            } else {
                LED_Off(1); /* Turn off PA0 breathing */
            }
        } else if (key_pressed == KEY2_PRESS) {
            /* For testing: Reset sensor initializations if they failed */
            if (test_aht20_init_status != 0) {
                test_aht20_init_status = AHT20_Init();
            }
            if (test_bmp280_init_status != 0) {
                test_bmp280_init_status = BMP280_Init();
            }
        }

        /* 4. Render LED outputs based on Mode */
        if (system_mode == 0) {
            /* Normal Mode: Update breathing effect on PA0 */
            LED_ProcessBreathing();
        } else {
            /* Anomaly Mode: Flash PC13 at high frequency (approx 5Hz)
               Since loop cycles every 10ms, toggle PC13 every 10 cycles (100ms) */
            flash_counter++;
            if (flash_counter >= 10) {
                flash_counter = 0;
                LED_Toggle(0);
            }
        }

        /* 5. Read Sensors at a slower rate (approx every 1.5 seconds)
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
        }

        /* Base loop delay of ~10ms (depends on CPU clock frequency of 72MHz) */
        Delay(72000);
    }
}

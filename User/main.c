#include "main.h"
#include "led.h"
#include "aht20.h"
#include "bmp280.h"

/* Global test variables for debugging */
volatile float test_aht20_temp = 0.0f;
volatile float test_aht20_humi = 0.0f;
volatile float test_bmp280_temp = 0.0f;
volatile float test_bmp280_press = 0.0f;

volatile uint8_t test_aht20_init_status = 1;  /* 0 = Success, 1 = Fail */
volatile uint8_t test_bmp280_init_status = 1; /* 0 = Success, 1 = Fail */

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
    /* 1. Initialize configured LEDs */
    LED_Init();

    /* 2. Initialize Sensors (PB6=SCL, PB7=SDA) */
    test_aht20_init_status = AHT20_Init();
    test_bmp280_init_status = BMP280_Init();

    /* Main Loop */
    while (1)
    {
        /* 3. Read AHT20 if initialized successfully */
        if (test_aht20_init_status == 0) {
            float temp = 0.0f;
            float humi = 0.0f;
            if (AHT20_ReadData(&temp, &humi) == 0) {
                test_aht20_temp = temp;
                test_aht20_humi = humi;
            }
        }

        /* 4. Read BMP280 if initialized successfully */
        if (test_bmp280_init_status == 0) {
            float temp = 0.0f;
            float press = 0.0f;
            if (BMP280_ReadData(&temp, &press) == 0) {
                test_bmp280_temp = temp;
                test_bmp280_press = press;
            }
        }

        /* 5. Heartbeat indications (Toggle LEDs) */
        LED_Toggle(0); /* Toggle PC13 */
        LED_Toggle(1); /* Toggle PA0 */

        /* Delay approx 1-2 seconds between read cycles */
        Delay(0x1FFFFF);
    }
}

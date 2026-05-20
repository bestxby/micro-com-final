#include "main.h"
#include "led.h"

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
    /* Initialize all configured LEDs */
    LED_Init();

    /* Main loop */
    while (1)
    {
        /* Toggle LED1 (PC13 by default, active low) */
        LED_Toggle(0);
        
        /* Toggle LED2 (PA0 by default, active high) */
        LED_Toggle(1);

        /* Software delay of approx 500ms depending on CPU clock */
        Delay(0x7FFFF);
    }
}

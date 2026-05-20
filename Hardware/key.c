#include "key.h"

/* Software millisecond delay for debouncing */
static void KEY_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/**
  * @brief  Initializes the keys (PB0, PB8) as inputs with pull-up resistors.
  * @param  None
  * @retval None
  */
void KEY_Init(void)
{
    /* 1. Enable GPIOB Clock */
    RCC->APB2ENR |= KEY1_RCC_ENR;
    (void)RCC->APB2ENR; /* Flush pipeline */

    /* 2. Configure PB0 as Input with pull-up/pull-down (CNF=10, MODE=00 -> 0x8) */
    GPIOB->CRL &= ~0x0000000F;
    GPIOB->CRL |=  0x00000008;

    /* 3. Configure PB8 as Input with pull-up/pull-down (CNF=10, MODE=00 -> 0x8) */
    GPIOB->CRH &= ~0x0000000F;
    GPIOB->CRH |=  0x00000008;

    /* 4. Select Pull-Up by setting ODR bits high */
    GPIOB->ODR |= KEY1_PIN;
    GPIOB->ODR |= KEY2_PIN;
}

/**
  * @brief  Scans the keys for state changes with software debounce.
  * @param  mode: 0 = Single-press mode (must release before triggering again),
  *               1 = Continuous mode (keeps triggering if held down).
  * @retval KEY1_PRESS, KEY2_PRESS, or KEY_NONE.
  */
uint8_t KEY_Scan(uint8_t mode)
{
    static uint8_t key_up = 1; /* Tracks whether keys have been released */
    
    if (mode) {
        key_up = 1;
    }

    /* Read pin states (0 = pressed, 1 = released due to pull-up) */
    uint8_t key1_low = (GPIOB->IDR & KEY1_PIN) ? 0 : 1;
    uint8_t key2_low = (GPIOB->IDR & KEY2_PIN) ? 0 : 1;

    if (key_up && (key1_low || key2_low)) {
        KEY_DelayMs(20); /* Wait 20ms to debounce */
        key_up = 0;      /* Block further triggers until released */
        
        /* Read again to confirm */
        key1_low = (GPIOB->IDR & KEY1_PIN) ? 0 : 1;
        key2_low = (GPIOB->IDR & KEY2_PIN) ? 0 : 1;

        if (key1_low) {
            return KEY1_PRESS;
        }
        if (key2_low) {
            return KEY2_PRESS;
        }
    } else if (!key1_low && !key2_low) {
        key_up = 1; /* Reset release flag when both buttons are released */
    }

    return KEY_NONE;
}

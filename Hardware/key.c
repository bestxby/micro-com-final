#include "key.h"

/* 按键消抖软件毫秒级延时函数 */
static void KEY_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/**
  * @brief  将按键引脚 (PB0, PB8) 初始化为带上拉的输入模式。
  * @param  无
  * @retval 无
  */
void KEY_Init(void)
{
    /* 1. 开启 GPIOA 与 GPIOB 端口外设时钟 */
    RCC->APB2ENR |= (KEY1_RCC_ENR | RCC_APB2ENR_IOPAEN);
    (void)RCC->APB2ENR; /* 刷新流水线 */

    /* 2. 配置 PB0 为上拉输入模式 (CNF=10, MODE=00 -> 0x8) */
    GPIOB->CRL &= ~0x0000000F;
    GPIOB->CRL |=  0x00000008;

    /* 3. 配置 PB8 为上拉输入模式 (CNF=10, MODE=00 -> 0x8) */
    GPIOB->CRH &= ~0x0000000F;
    GPIOB->CRH |=  0x00000008;

    /* 4. 配置 PA12 和 PA15 为上拉输入模式 (CNF=10, MODE=00 -> 0x8) */
    GPIOA->CRH &= ~0xF00F0000;
    GPIOA->CRH |=  0x80080000;

    /* 5. 将 ODR 寄存器对应位置 1 选定为上拉输入模式 */
    GPIOB->ODR |= KEY1_PIN;
    GPIOB->ODR |= KEY2_PIN;
    GPIOA->ODR |= (GPIO_Pin_12 | GPIO_Pin_15);
}

/**
  * @brief  带软件消抖的按键状态扫描函数。
  * @param  mode: 0 = 单次触发模式（按键必须释放后才能触发下一次）,
  *               1 = 连续触发模式（按下不放可以持续触发）。
  * @retval KEY1_PRESS, KEY2_PRESS, KEY_LEFT_PRESS, KEY_RIGHT_PRESS, 或 KEY_NONE。
  */
uint8_t KEY_Scan(uint8_t mode)
{
    static uint8_t key_up = 1; /* 按键释放标志位 */
    
    if (mode) {
        key_up = 1;
    }

    /* 读取物理引脚输入电平 (低电平表示按下，高电平表示释放) */
    uint8_t key1_low = (GPIOB->IDR & KEY1_PIN) ? 0 : 1;
    uint8_t key2_low = (GPIOB->IDR & KEY2_PIN) ? 0 : 1;
    uint8_t key3_low = (GPIOA->IDR & GPIO_Pin_15) ? 0 : 1; // Left: PA15
    uint8_t key4_low = (GPIOA->IDR & GPIO_Pin_12) ? 0 : 1; // Right: PA12

    if (key_up && (key1_low || key2_low || key3_low || key4_low)) {
        KEY_DelayMs(20); /* 延时 20ms 进行消抖 */
        key_up = 0;      /* 锁定，防止重复触发 */
        
        /* 再次读取确认按键状态 */
        key1_low = (GPIOB->IDR & KEY1_PIN) ? 0 : 1;
        key2_low = (GPIOB->IDR & KEY2_PIN) ? 0 : 1;
        key3_low = (GPIOA->IDR & GPIO_Pin_15) ? 0 : 1;
        key4_low = (GPIOA->IDR & GPIO_Pin_12) ? 0 : 1;

        if (key1_low) {
            return KEY1_PRESS;
        }
        if (key2_low) {
            return KEY2_PRESS;
        }
        if (key3_low) {
            return KEY_LEFT_PRESS;
        }
        if (key4_low) {
            return KEY_RIGHT_PRESS;
        }
    } else if (!key1_low && !key2_low && !key3_low && !key4_low) {
        key_up = 1; /* 所有按键均释放后，复位释放标志位 */
    }

    return KEY_NONE;
}

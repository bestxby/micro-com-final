#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

/* 按键扫描状态返回值 */
#define KEY_NONE        0   /* 无按键按下 */
#define KEY1_PRESS      1   /* KEY1 (PB0) 按下 - 中间 */
#define KEY2_PRESS      2   /* KEY2 (PB8) 按下 - 上 */
#define KEY_LEFT_PRESS  3   /* 左键 (PA15) 按下 */
#define KEY_RIGHT_PRESS 4   /* 右键 (PA12) 按下 */

/* 按键硬件端口及引脚定义 */
#define KEY1_PORT       GPIOB
#define KEY1_PIN        GPIO_Pin_0
#define KEY1_RCC_ENR    RCC_APB2ENR_IOPBEN

#define KEY2_PORT       GPIOB
#define KEY2_PIN        GPIO_Pin_8
#define KEY2_RCC_ENR    RCC_APB2ENR_IOPBEN

/* 外部公开接口 */
void KEY_Init(void);
uint8_t KEY_Scan(uint8_t mode);

#endif /* __KEY_H */

#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

/* Key definitions */
#define KEY_NONE        0
#define KEY1_PRESS      1
#define KEY2_PRESS      2

/* Pin Definitions */
#define KEY1_PORT       GPIOB
#define KEY1_PIN        GPIO_Pin_0
#define KEY1_RCC_ENR    RCC_APB2ENR_IOPBEN

#define KEY2_PORT       GPIOB
#define KEY2_PIN        GPIO_Pin_8
#define KEY2_RCC_ENR    RCC_APB2ENR_IOPBEN

/* Public API */
void KEY_Init(void);
uint8_t KEY_Scan(uint8_t mode);

#endif /* __KEY_H */

#ifndef __LED_H
#define __LED_H

#include <stdint.h>

/* ============================================================
 * LED 硬件引脚资源分配宏定义
 * 可根据实际开发板布线修改以下宏值
 * ============================================================ */

/* ---- LED1 (默认PC13: 核心板载 LED，低电平有效) ---- */
#define LED1_PORT             GPIOC
#define LED1_PIN              13
#define LED1_RCC_ENR          RCC_APB2ENR_IOPCEN
#define LED1_ACTIVE_LEVEL     0   /* 0 = 低电平点亮, 1 = 高电平点亮 */

/* ---- LED2 (默认PA0: 对应 TIM2 通道 1 用于 PWM 呼吸灯) ---- */
#define LED2_PORT             GPIOA
#define LED2_PIN              0
#define LED2_RCC_ENR          RCC_APB2ENR_IOPAEN
#define LED2_ACTIVE_LEVEL     1   /* 1 = 高电平点亮 */

/* ---- 控制灯的总数 ---- */
#define LED_COUNT             2

/* ============================================================
 * 外部公开接口
 * ============================================================ */

void LED_Init(void);
void LED_On(uint8_t index);
void LED_Off(uint8_t index);
void LED_Toggle(uint8_t index);
void LED_Write(uint8_t index, uint8_t state);

/* 呼吸灯及异常闪烁控制公开接口 */
void LED_SetBreathingDuty(uint16_t duty);
void LED_ProcessBreathing(void);

#endif /* __LED_H */

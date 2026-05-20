#ifndef __LED_H
#define __LED_H

#include <stdint.h>

/* ============================================================
 * LED Pin Configuration
 * Modify these macros to match your board wiring
 * ============================================================ */

/* ---- LED1 (default: PC13, common on Blue Pill / minimum system boards) ---- */
#define LED1_PORT             GPIOC
#define LED1_PIN              13
#define LED1_RCC_ENR          RCC_APB2ENR_IOPCEN
#define LED1_ACTIVE_LEVEL     0   /* 0 = active low (sink current), 1 = active high */

/* ---- LED2 (default: PA0, change as needed) ---- */
#define LED2_PORT             GPIOA
#define LED2_PIN              0
#define LED2_RCC_ENR          RCC_APB2ENR_IOPAEN
#define LED2_ACTIVE_LEVEL     1   /* 1 = active high (source current) */

/* ---- LED3 (default: PA1, change as needed) ---- */
#define LED3_PORT             GPIOA
#define LED3_PIN              1
#define LED3_RCC_ENR          RCC_APB2ENR_IOPAEN
#define LED3_ACTIVE_LEVEL     1

/* ---- LED4 (default: PA2, change as needed) ---- */
#define LED4_PORT             GPIOA
#define LED4_PIN              2
#define LED4_RCC_ENR          RCC_APB2ENR_IOPAEN
#define LED4_ACTIVE_LEVEL     1

/* ---- LED count ---- */
#define LED_COUNT             4

/* ============================================================
 * Public API
 * ============================================================ */

void LED_Init(void);
void LED_On(uint8_t index);
void LED_Off(uint8_t index);
void LED_Toggle(uint8_t index);
void LED_Write(uint8_t index, uint8_t state);

#endif /* __LED_H */

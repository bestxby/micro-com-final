#include "led.h"
#include "stm32f10x.h"

/* ---- Per-LED descriptor table ---- */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint32_t      rcc_enr;
    uint8_t       active_level;
} LED_Desc;

static const LED_Desc led_table[LED_COUNT] = {
    { LED1_PORT, (1U << LED1_PIN), LED1_RCC_ENR, LED1_ACTIVE_LEVEL },
    { LED2_PORT, (1U << LED2_PIN), LED2_RCC_ENR, LED2_ACTIVE_LEVEL },
    { LED3_PORT, (1U << LED3_PIN), LED3_RCC_ENR, LED3_ACTIVE_LEVEL },
    { LED4_PORT, (1U << LED4_PIN), LED4_RCC_ENR, LED4_ACTIVE_LEVEL },
};

/* ---- Turn on a single RCC clock (idempotent bit-band write) ---- */
static void rcc_enable(uint32_t enr)
{
    RCC->APB2ENR |= enr;
    (void)RCC->APB2ENR; /* force APB2 pipeline flush */
}

/* ---- Configure pin as 50 MHz push-pull output ---- */
static void gpio_config_output(const LED_Desc *led)
{
    uint32_t pin  = led->pin;
    uint32_t cr   = (uint32_t)&led->port->CRL;

    /* CRL for pin 0..7, CRH for pin 8..15 */
    if (led->pin > 0x00FF) {
        cr = (uint32_t)&led->port->CRH;
        pin >>= 8;
    }

    /* locate the 4-bit CNF+MODE field and shift the config word in */
    uint32_t pos   = (pin & 0x7U) * 4U;
    uint32_t field = 0x3U;          /* CNF=00 MODE=11 → 50 MHz push-pull */
    uint32_t mask  = 0xFU << pos;

    __IO uint32_t *reg = (__IO uint32_t *)cr;
    *reg = (*reg & ~mask) | (field << pos);
}

/* ============================================================
 * LED_Init — enable clocks and configure all LED GPIO pins
 * ============================================================ */
void LED_Init(void)
{
    /* track already-enabled ports to avoid redundant register writes */
    uint32_t enabled = 0;

    for (uint8_t i = 0; i < LED_COUNT; i++) {
        if (!(enabled & led_table[i].rcc_enr)) {
            rcc_enable(led_table[i].rcc_enr);
            enabled |= led_table[i].rcc_enr;
        }
        gpio_config_output(&led_table[i]);
        LED_Off(i);  /* default to off after init */
    }
}

/* ---- Internal: write a pin to its active level ---- */
static inline void led_set_pin(const LED_Desc *led)
{
    if (led->active_level) {
        led->port->BSRR = led->pin;
    } else {
        led->port->BRR  = led->pin;
    }
}

/* ---- Internal: write a pin to its inactive level ---- */
static inline void led_reset_pin(const LED_Desc *led)
{
    if (led->active_level) {
        led->port->BRR  = led->pin;
    } else {
        led->port->BSRR = led->pin;
    }
}

/* ============================================================
 * LED_On — turn a single LED on
 * ============================================================ */
void LED_On(uint8_t index)
{
    if (index >= LED_COUNT) return;
    led_set_pin(&led_table[index]);
}

/* ============================================================
 * LED_Off — turn a single LED off
 * ============================================================ */
void LED_Off(uint8_t index)
{
    if (index >= LED_COUNT) return;
    led_reset_pin(&led_table[index]);
}

/* ============================================================
 * LED_Toggle — flip a single LED
 * ============================================================ */
void LED_Toggle(uint8_t index)
{
    if (index >= LED_COUNT) return;
    const LED_Desc *led = &led_table[index];

    /* read ODR, check if the pin is "on" at the electrical level */
    if (led->port->ODR & led->pin) {
        led_reset_pin(led);
    } else {
        led_set_pin(led);
    }
}

/* ============================================================
 * LED_Write — set LED to explicit state (0 = off, non-zero = on)
 * ============================================================ */
void LED_Write(uint8_t index, uint8_t state)
{
    if (index >= LED_COUNT) return;
    if (state) {
        led_set_pin(&led_table[index]);
    } else {
        led_reset_pin(&led_table[index]);
    }
}

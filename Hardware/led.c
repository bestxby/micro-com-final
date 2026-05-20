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
        
        if (i == 1) {
            /* LED2 (PA0) is TIM2_CH1 PWM output.
               We configure PA0 as Alternate Function Push-Pull (CNF=10, MODE=11 -> 0xB) */
            GPIOA->CRL &= ~0x0000000F;
            GPIOA->CRL |=  0x0000000B;
        } else {
            gpio_config_output(&led_table[i]);
        }
        
        LED_Off(i);  /* default to off after init */
    }

    /* Configure TIM2 for CH1 PWM output */
    /* 1. Enable TIM2 clock on APB1 */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR; /* Flush pipeline */

    /* 2. Configure Prescaler and Auto-reload for 720 Hz frequency
          PSC = 999 (72MHz / 1000 = 72kHz tick rate)
          ARR = 99 (Period = 100 ticks -> 720Hz PWM frequency) */
    TIM2->PSC = 999;
    TIM2->ARR = 99;

    /* 3. Configure Capture/Compare Mode Register 1 (CCMR1)
          OC1M = 110 (PWM mode 1)
          OC1PE = 1 (Enable preload register for channel 1) */
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (6U << 4) | TIM_CCMR1_OC1PE;

    /* 4. Enable Output Channel 1 in Capture/Compare Enable Register (CCER) */
    TIM2->CCER |= TIM_CCER_OC1E;

    /* 5. Enable TIM2 Counter in Control Register 1 (CR1) */
    TIM2->CR1 |= TIM_CR1_CEN;

    /* 6. Set initial duty cycle to 0 (off) */
    TIM2->CCR1 = 0;
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
    if (index == 1) {
        TIM2->CCR1 = 99; /* Max duty cycle */
    } else {
        led_set_pin(&led_table[index]);
    }
}

/* ============================================================
 * LED_Off — turn a single LED off
 * ============================================================ */
void LED_Off(uint8_t index)
{
    if (index >= LED_COUNT) return;
    if (index == 1) {
        TIM2->CCR1 = 0; /* Min duty cycle */
    } else {
        led_reset_pin(&led_table[index]);
    }
}

/* ============================================================
 * LED_Toggle — flip a single LED
 * ============================================================ */
void LED_Toggle(uint8_t index)
{
    if (index >= LED_COUNT) return;
    if (index == 1) {
        if (TIM2->CCR1 > 0) {
            TIM2->CCR1 = 0;
        } else {
            TIM2->CCR1 = 99;
        }
    } else {
        const LED_Desc *led = &led_table[index];
        /* toggle the physical state of the pin directly */
        if (led->port->ODR & led->pin) {
            led->port->BRR = led->pin;
        } else {
            led->port->BSRR = led->pin;
        }
    }
}

/* ============================================================
 * LED_Write — set LED to explicit state (0 = off, non-zero = on)
 * ============================================================ */
void LED_Write(uint8_t index, uint8_t state)
{
    if (index >= LED_COUNT) return;
    if (state) {
        LED_On(index);
    } else {
        LED_Off(index);
    }
}

/* ============================================================
 * LED_SetBreathingDuty — write duty cycle directly to TIM2 CCR1
 * ============================================================ */
void LED_SetBreathingDuty(uint16_t duty)
{
    if (duty > 99) {
        duty = 99;
    }
    TIM2->CCR1 = duty;
}

/* ============================================================
 * LED_ProcessBreathing — smooth duty cycle transition logic
 * ============================================================ */
void LED_ProcessBreathing(void)
{
    static int16_t duty = 0;
    static int16_t dir = 1;

    duty += dir;
    if (duty >= 99) {
        duty = 99;
        dir = -1;
    } else if (duty <= 0) {
        duty = 0;
        dir = 1;
    }
    
    TIM2->CCR1 = duty;
}

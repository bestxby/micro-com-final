#include "led.h"
#include "stm32f10x.h"

/* ---- LED 物理硬件描述体结构 ---- */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint32_t      rcc_enr;
    uint8_t       active_level;
} LED_Desc;

/* LED 设备物理映射描述表 */
static const LED_Desc led_table[LED_COUNT] = {
    { LED1_PORT, (1U << LED1_PIN), LED1_RCC_ENR, LED1_ACTIVE_LEVEL },
    { LED2_PORT, (1U << LED2_PIN), LED2_RCC_ENR, LED2_ACTIVE_LEVEL },
    { LED3_PORT, (1U << LED3_PIN), LED3_RCC_ENR, LED3_ACTIVE_LEVEL },
    { LED4_PORT, (1U << LED4_PIN), LED4_RCC_ENR, LED4_ACTIVE_LEVEL },
};

/* ---- 开启指定的 GPIO 端口时钟 (幂等写入) ---- */
static void rcc_enable(uint32_t enr)
{
    RCC->APB2ENR |= enr;
    (void)RCC->APB2ENR; /* 强刷 APB2 总线流水线 */
}

/* ---- 配置指定 GPIO 引脚为 50 MHz 推挽输出模式 ---- */
static void gpio_config_output(const LED_Desc *led)
{
    uint32_t pin_mask = led->pin;
    uint32_t pin_index = 0;
    while (pin_mask > 1) {
        pin_mask >>= 1;
        pin_index++;
    }

    uint32_t cr = (uint32_t)&led->port->CRL;

    /* 0..7号引脚对应 CRL 寄存器, 8..15号引脚对应 CRH 寄存器 */
    if (pin_index >= 8) {
        cr = (uint32_t)&led->port->CRH;
        pin_index -= 8;
    }

    /* 准确定位 4 位控制字段并执行位修改 */
    uint32_t pos   = pin_index * 4U;
    uint32_t field = 0x3U;          /* CNF=00 MODE=11 -> 50 MHz 推挽输出 */
    uint32_t mask  = 0xFU << pos;

    __IO uint32_t *reg = (__IO uint32_t *)cr;
    *reg = (*reg & ~mask) | (field << pos);
}

/* ============================================================
 * LED_Init — 初始化所有配置的 LED 控制端口并配置 TIM2 PWM
 * ============================================================ */
void LED_Init(void)
{
    /* 1. 初始化 PC13 (LED1, 核心板载 LED) 为标准 GPIO 输出 */
    rcc_enable(led_table[0].rcc_enr);
    gpio_config_output(&led_table[0]);
    LED_Off(0);

    /* 2. 开启 TIM2 和 GPIOA 时钟 */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    (void)RCC->APB2ENR;

    /* 3. 配置 PA0, PA1, PA2 为复用推挽输出 50MHz (CNF=10, MODE=11) */
    GPIOA->CRL &= ~0x00000FFF;
    GPIOA->CRL |=  0x00000BBB;

    /* 4. 配置 TIM2 寄存器参数以产生约 1kHz PWM */
    TIM2->PSC = 719; // 72MHz / 720 = 100kHz clock
    TIM2->ARR = 99;  // 100kHz / 100 = 1kHz PWM frequency

    // 配置通道 1, 2, 3 为 PWM 模式 1 (OCxM = 110) 且使能预装载
    TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    TIM2->CCMR1 |= (6 << 4) | (6 << 12);
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;

    TIM2->CCMR2 &= ~TIM_CCMR2_OC3M;
    TIM2->CCMR2 |= (6 << 4);
    TIM2->CCMR2 |= TIM_CCMR2_OC3PE;

    // 使能通道 1, 2, 3 输出比较
    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

    // 启用 TIM2 主计数器
    TIM2->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
    TIM2->EGR = TIM_EGR_UG;

    // 默认关闭所有通道 (CCR = 0)
    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;
    TIM2->CCR3 = 0;
}

/* ============================================================
 * LED_On — 点亮指定的 LED
 * ============================================================ */
void LED_On(uint8_t index)
{
    if (index == 1) {
        TIM2->CCR1 = 99; // PA0
    } else if (index == 2) {
        TIM2->CCR2 = 99; // PA1
    } else if (index == 3) {
        TIM2->CCR3 = 99; // PA2
    } else if (index == 0) {
        if (led_table[0].active_level) {
            led_table[0].port->BSRR = led_table[0].pin;
        } else {
            led_table[0].port->BRR  = led_table[0].pin;
        }
    }
}

/* ============================================================
 * LED_Off — 熄灭指定的 LED
 * ============================================================ */
void LED_Off(uint8_t index)
{
    if (index == 1) {
        TIM2->CCR1 = 0; // PA0
    } else if (index == 2) {
        TIM2->CCR2 = 0; // PA1
    } else if (index == 3) {
        TIM2->CCR3 = 0; // PA2
    } else if (index == 0) {
        if (led_table[0].active_level) {
            led_table[0].port->BRR  = led_table[0].pin;
        } else {
            led_table[0].port->BSRR = led_table[0].pin;
        }
    }
}

/* ============================================================
 * LED_Toggle — 翻转指定的 LED 状态
 * ============================================================ */
void LED_Toggle(uint8_t index)
{
    if (index == 1) {
        TIM2->CCR1 = (TIM2->CCR1 > 0) ? 0 : 99;
    } else if (index == 2) {
        TIM2->CCR2 = (TIM2->CCR2 > 0) ? 0 : 99;
    } else if (index == 3) {
        TIM2->CCR3 = (TIM2->CCR3 > 0) ? 0 : 99;
    } else if (index == 0) {
        const LED_Desc *led = &led_table[0];
        if (led->port->ODR & led->pin) {
            led->port->BRR = led->pin;
        } else {
            led->port->BSRR = led->pin;
        }
    }
}

/* ============================================================
 * LED_Write — 根据输入状态强置亮灭 (0 = 灭, 非零 = 亮)
 * ============================================================ */
void LED_Write(uint8_t index, uint8_t state)
{
    if (state) {
        LED_On(index);
    } else {
        LED_Off(index);
    }
}

/* ============================================================
 * LED_SetBreathingDuty — 直接写入 TIM2 通道 3 (绿灯) 的 PWM 占空比
 * ============================================================ */
void LED_SetBreathingDuty(uint16_t duty)
{
    if (duty > 99) {
        duty = 99;
    }
    TIM2->CCR3 = duty;
}

/* ============================================================
 * LED_ProcessBreathing — 平滑增减 PWM 占空比的呼吸处理函数 (作用于绿灯)
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
    
    TIM2->CCR3 = duty;
}

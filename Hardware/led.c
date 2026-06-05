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
    uint32_t pin  = led->pin;
    uint32_t cr   = (uint32_t)&led->port->CRL;

    /* 0..7号引脚对应 CRL 寄存器, 8..15号引脚对应 CRH 寄存器 */
    if (led->pin > 0x00FF) {
        cr = (uint32_t)&led->port->CRH;
        pin >>= 8;
    }

    /* 准确定位 4 位控制字段并执行位修改 */
    uint32_t pos   = (pin & 0x7U) * 4U;
    uint32_t field = 0x3U;          /* CNF=00 MODE=11 -> 50 MHz 推挽输出 */
    uint32_t mask  = 0xFU << pos;

    __IO uint32_t *reg = (__IO uint32_t *)cr;
    *reg = (*reg & ~mask) | (field << pos);
}

/* ============================================================
 * LED_Init — 初始化所有配置的 LED 控制端口
 * ============================================================ */
void LED_Init(void)
{
    /* 跟踪已启用的时钟以避免重复写寄存器 */
    uint32_t enabled = 0;

    for (uint8_t i = 0; i < LED_COUNT; i++) {
        if (!(enabled & led_table[i].rcc_enr)) {
            rcc_enable(led_table[i].rcc_enr);
            enabled |= led_table[i].rcc_enr;
        }
        
        gpio_config_output(&led_table[i]);
        LED_Off(i);  /* 初始化完毕默认关闭 */
    }
}

/* ---- 内部函数: 将 GPIO 电平写入引脚的“有效激活”电平 ---- */
static __inline void led_set_pin(const LED_Desc *led)
{
    if (led->active_level) {
        led->port->BSRR = led->pin;
    } else {
        led->port->BRR  = led->pin;
    }
}

/* ---- 内部函数: 将 GPIO 电平写入引脚的“无效关闭”电平 ---- */
static __inline void led_reset_pin(const LED_Desc *led)
{
    if (led->active_level) {
        led->port->BRR  = led->pin;
    } else {
        led->port->BSRR = led->pin;
    }
}

/* ============================================================
 * LED_On — 点亮指定的 LED
 * ============================================================ */
void LED_On(uint8_t index)
{
    if (index >= LED_COUNT) return;
    led_set_pin(&led_table[index]);
}

/* ============================================================
 * LED_Off — 熄灭指定的 LED
 * ============================================================ */
void LED_Off(uint8_t index)
{
    if (index >= LED_COUNT) return;
    led_reset_pin(&led_table[index]);
}

/* ============================================================
 * LED_Toggle — 翻转指定的 LED 状态
 * ============================================================ */
void LED_Toggle(uint8_t index)
{
    if (index >= LED_COUNT) return;
    const LED_Desc *led = &led_table[index];
    /* 直接读写 ODR 寄存器物理电平进行逻辑翻转 */
    if (led->port->ODR & led->pin) {
        led->port->BRR = led->pin;
    } else {
        led->port->BSRR = led->pin;
    }
}

/* ============================================================
 * LED_Write — 根据输入状态强置高低电平 (0 = 灭, 非零 = 亮)
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
 * LED_SetBreathingDuty — 直接写入 TIM2 通道 1 寄存器的 PWM 占空比
 * ============================================================ */
void LED_SetBreathingDuty(uint16_t duty)
{
    if (duty > 99) {
        duty = 99;
    }
    TIM2->CCR1 = duty;
}

/* ============================================================
 * LED_ProcessBreathing — 平滑增减 PWM 占空比的呼吸处理函数
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

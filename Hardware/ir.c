#include "ir.h"
#include "stm32f10x.h"

// State variables for NEC decoding
#define IR_STATE_IDLE         0
#define IR_STATE_LEADER_LOW   1
#define IR_STATE_DATA         2

static volatile uint8_t  ir_state = IR_STATE_IDLE;
static volatile uint32_t ir_raw_data = 0;
static volatile uint8_t  ir_bit_count = 0;

static volatile uint8_t  ir_received_flag = 0;
static volatile uint8_t  ir_received_code = 0;
static volatile uint8_t  ir_repeat_flag = 0;

void IR_Init(void)
{
    // 1. Enable GPIOC, AFIO and TIM4 clocks
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    (void)RCC->APB2ENR; // Refresh pipeline

    // 2. Configure PC13 as Input Pull-Up (CNF=10, MODE=00 -> 0x8)
    // PC13 resides in CRH register, bits 20..23
    GPIOC->CRH &= ~0x00F00000;
    GPIOC->CRH |=  0x00800000;
    GPIOC->ODR |= (1 << 13); // Enable internal pull-up

    // 3. Map PC13 to EXTI13 (AFIO->EXTICR[3], bits 4..7 to 0x2 for GPIOC)
    AFIO->EXTICR[3] &= ~0x000000F0;
    AFIO->EXTICR[3] |=  0x00000020;

    // 4. Configure EXTI Line 13 for dual-edge trigger
    EXTI->IMR |= (1 << 13);  // Interrupt Mask Register (Enable)
    EXTI->RTSR |= (1 << 13); // Rising Trigger Selection Register
    EXTI->FTSR |= (1 << 13); // Falling Trigger Selection Register
    EXTI->PR = (1 << 13);    // Clear pending flag

    // 5. Configure TIM4 to count at 1MHz (1us per tick)
    TIM4->PSC = 71;       // 72MHz / 72 = 1MHz
    TIM4->ARR = 0xFFFF;   // Maximum range ~65.5ms
    TIM4->CNT = 0;
    TIM4->CR1 |= TIM_CR1_CEN; // Enable counter

    // 6. Enable NVIC for EXTI15_10 Interrupt (Priority 2, 2)
    NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(4, 2, 2));
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

uint8_t IR_GetData(uint8_t *code)
{
    if (ir_received_flag) {
        *code = ir_received_code;
        ir_received_flag = 0;
        return 1;
    }
    return 0;
}

// EXTI Line 15..10 Interrupt Handler (PC13 uses this vector)
void EXTI15_10_IRQHandler(void)
{
    // Check if interrupt is from Line 13
    if (EXTI->PR & (1 << 13)) {
        EXTI->PR = (1 << 13); // Clear EXTI Line 13 pending flag

        uint32_t duration = TIM4->CNT;
        TIM4->CNT = 0; // Reset counter for the next interval

        uint8_t pin_state = (GPIOC->IDR & (1 << 13)) ? 1 : 0;

        if (pin_state) {
            // Rising edge: preceding pulse was LOW. Measure LOW pulse duration.
            if (ir_state == IR_STATE_IDLE || ir_state == IR_STATE_LEADER_LOW) {
                // Leader LOW is 9000us
                if (duration >= 8000 && duration <= 10000) {
                    ir_state = IR_STATE_LEADER_LOW;
                } else {
                    ir_state = IR_STATE_IDLE;
                }
            } else if (ir_state == IR_STATE_DATA) {
                // Data bit LOW is 560us
                if (duration >= 350 && duration <= 800) {
                    // Valid data bit start, keep state as IR_STATE_DATA
                } else {
                    ir_state = IR_STATE_IDLE; // Error
                }
            }
        } else {
            // Falling edge: preceding pulse was HIGH. Measure HIGH pulse duration.
            if (ir_state == IR_STATE_LEADER_LOW) {
                // Leader HIGH is 4500us
                if (duration >= 3800 && duration <= 5200) {
                    ir_state = IR_STATE_DATA;
                    ir_bit_count = 0;
                    ir_raw_data = 0;
                }
                // Repeat code HIGH is 2250us
                else if (duration >= 1800 && duration <= 2600) {
                    ir_repeat_flag = 1;
                    ir_state = IR_STATE_IDLE;
                } else {
                    ir_state = IR_STATE_IDLE;
                }
            } else if (ir_state == IR_STATE_DATA) {
                // Data bit HIGH is either 560us (bit '0') or 1680us (bit '1')
                if (duration >= 350 && duration <= 800) {
                    // Bit '0'
                    ir_raw_data >>= 1;
                    ir_bit_count++;
                } else if (duration >= 1300 && duration <= 2000) {
                    // Bit '1'
                    ir_raw_data = (ir_raw_data >> 1) | 0x80000000;
                    ir_bit_count++;
                } else {
                    ir_state = IR_STATE_IDLE; // Error
                }

                if (ir_bit_count == 32) {
                    // Received all 32 bits
                    uint8_t cmd = (ir_raw_data >> 16) & 0xFF;
                    uint8_t cmd_inv = (ir_raw_data >> 24) & 0xFF;
                    if (cmd == (uint8_t)(~cmd_inv)) {
                        ir_received_code = cmd;
                        ir_received_flag = 1;
                    }
                    ir_state = IR_STATE_IDLE;
                }
            }
        }
    }
}

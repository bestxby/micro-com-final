#include "sd.h"

/**
  * @brief  Initialize USART2 on PA2 (TX) and PA3 (RX)
  * @param  baudrate: Target speed (e.g. 115200)
  * @retval None
  */
void SD_UART_Init(uint32_t baudrate)
{
    /* 1. 开启 GPIOA 和 USART2 时钟 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB2ENR; // Refresh pipeline
    
    /* 2. 配置 GPIO 引脚
     *    PA2 (TX): Alternate Function Push-Pull 50MHz (CNF=10, MODE=11 -> 0xB)
     *    PA3 (RX): Input with Pull-Up (CNF=10, MODE=00 -> 0x8)
     */
    GPIOA->CRL &= ~0x0000FF00;
    GPIOA->CRL |=  0x00008B00;
    
    /* Enable Pull-Up on PA3 by writing 1 to ODR */
    GPIOA->ODR |= GPIO_Pin_3;
    
    /* 3. Compute and set USART Baud Rate (USART2 is on APB1, 36MHz) */
    uint32_t apb1_clk = 36000000;
    float divider = (float)apb1_clk / (16.0f * baudrate);
    uint16_t mantissa = (uint16_t)divider;
    uint16_t fraction = (uint16_t)((divider - mantissa) * 16.0f + 0.5f);
    USART2->BRR = (mantissa << 4) | (fraction & 0x0F);
    
    /* 4. Configure USART2:
     *    TE = 1 (Transmitter enable)
     *    RE = 1 (Receiver enable)
     *    UE = 1 (USART enable)
     */
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
    USART2->CR2 = 0;
    USART2->CR3 = 0;
}

/**
  * @brief  Send one character over USART2
  * @param  c: Character to transmit
  */
void SD_UART_SendChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

/**
  * @brief  Send null-terminated string over USART2
  * @param  str: Pointer to string
  */
void SD_UART_SendString(const char *str)
{
    while (*str) {
        SD_UART_SendChar(*str++);
    }
}

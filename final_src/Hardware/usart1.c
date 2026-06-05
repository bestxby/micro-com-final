#include "usart1.h"

/* Circular RX Buffer */
volatile char usart_rx_buf[USART_RX_BUF_SIZE];
volatile uint16_t usart_rx_head = 0;
volatile uint16_t usart_rx_tail = 0;

/**
  * @brief  Initialize USART1 at register level (PA9=TX, PA10=RX, APB2=72MHz)
  * @param  baudrate: Target speed (e.g. 115200)
  * @retval None
  */
void USART1_Init(uint32_t baudrate)
{
    /* 1. Enable GPIOA, USART1 and AFIO clock in APB2 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_AFIOEN;
    (void)RCC->APB2ENR; // Refresh pipeline
    
    /* 2. Configure GPIO pins
     *    PA9 (TX): Alternate Function Push-Pull 50MHz (CNF=10, MODE=11 -> 0xB)
     *    PA10 (RX): Input with Pull-Up/Down (CNF=10, MODE=00 -> 0x8)
     */
    GPIOA->CRH &= ~0x00000FF0;
    GPIOA->CRH |=  0x000008B0;
    
    /* Enable Pull-Up on PA10 by writing 1 to ODR */
    GPIOA->ODR |= GPIO_Pin_10;
    
    /* 3. Compute and set USART Baud Rate (USART1 is on APB2, 72MHz) */
    uint32_t apb2_clk = 72000000;
    float divider = (float)apb2_clk / (16.0f * baudrate);
    uint16_t mantissa = (uint16_t)divider;
    uint16_t fraction = (uint16_t)((divider - mantissa) * 16.0f + 0.5f);
    USART1->BRR = (mantissa << 4) | (fraction & 0x0F);
    
    /* 4. Configure USART1:
     *    TE = 1 (Transmitter enable)
     *    RE = 1 (Receiver enable)
     *    RXNEIE = 1 (RXNE interrupt enable)
     *    UE = 1 (USART enable)
     */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    
    /* 5. Configure NVIC for USART1 Interrupt */
    /* Set priority group 2, priority 0,0 for USART1 (highest responsive) */
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(4, 1, 1));
    NVIC_EnableIRQ(USART1_IRQn);
    
    /* Clear buffer indices */
    USART1_ClearBuffer();
}

/**
  * @brief  Send one character over USART1
  * @param  c: Character to transmit
  * @retval None
  */
void USART1_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

/**
  * @brief  Send null-terminated string over USART1
  * @param  str: Pointer to string
  * @retval None
  */
void USART1_SendString(const char *str)
{
    while (*str) {
        USART1_SendChar(*str++);
    }
}

/**
  * @brief  Read one character from the RX circular buffer
  * @param  c: Pointer to destination character
  * @retval 1 = Character read, 0 = Buffer empty
  */
uint8_t USART1_ReadChar(char *c)
{
    if (usart_rx_head == usart_rx_tail) {
        return 0; // Empty
    }
    *c = usart_rx_buf[usart_rx_tail];
    usart_rx_tail = (usart_rx_tail + 1) % USART_RX_BUF_SIZE;
    return 1;
}

/**
  * @brief  Clear the RX circular buffer
  * @param  None
  * @retval None
  */
void USART1_ClearBuffer(void)
{
    usart_rx_head = 0;
    usart_rx_tail = 0;
}

/**
  * @brief  USART1 Global Interrupt Handler
  *         Read incoming byte into RX circular buffer.
  */
void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE) {
        char c = (char)(USART1->DR & 0xFF);
        uint16_t next = (usart_rx_head + 1) % USART_RX_BUF_SIZE;
        if (next != usart_rx_tail) {
            usart_rx_buf[usart_rx_head] = c;
            usart_rx_head = next;
        }
    }
}

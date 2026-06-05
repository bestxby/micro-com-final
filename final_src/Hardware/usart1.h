#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"

/* Circular Buffer Size */
#define USART_RX_BUF_SIZE 512

/* Public API */
void USART1_Init(uint32_t baudrate);
void USART1_SendChar(char c);
void USART1_SendString(const char *str);
uint8_t USART1_ReadChar(char *c);
void USART1_ClearBuffer(void);

#endif /* __USART1_H */

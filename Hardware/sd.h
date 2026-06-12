#ifndef __SD_H
#define __SD_H

#include "stm32f10x.h"

/* 
 * SD 模块的通信已经迁移至硬件 USART2 透传给 Zynq，
 * 本文件由原 SPI 驱动完全重构为硬件串口驱动。
 */

void SD_UART_Init(uint32_t baudrate);
void SD_UART_SendChar(char c);
void SD_UART_SendString(const char *str);

#endif /* __SD_H */

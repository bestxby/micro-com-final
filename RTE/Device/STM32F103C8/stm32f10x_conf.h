#ifndef __STM32F10x_CONF_H
#define __STM32F10x_CONF_H

/* 标准外设库头文件包含 */
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_flash.h"
#include "misc.h"

/* 关闭全断言 (编译调试时可改为 #define USE_FULL_ASSERT 1) */
/* #define USE_FULL_ASSERT    1 */

#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif

#endif /* __STM32F10x_CONF_H */

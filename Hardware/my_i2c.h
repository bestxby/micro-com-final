#ifndef __MY_I2C_H
#define __MY_I2C_H

#include "stm32f10x.h"

/* I2C SCL & SDA Pin Definitions */
#define I2C_SCL_PORT        GPIOB
#define I2C_SCL_PIN         GPIO_Pin_6
#define I2C_SDA_PORT        GPIOB
#define I2C_SDA_PIN         GPIO_Pin_7
#define I2C_RCC_ENR         RCC_APB2ENR_IOPBEN

/* I2C GPIO physical write helpers */
#define I2C_SCL_H()         (I2C_SCL_PORT->BSRR = I2C_SCL_PIN)
#define I2C_SCL_L()         (I2C_SCL_PORT->BRR  = I2C_SCL_PIN)
#define I2C_SDA_H()         (I2C_SDA_PORT->BSRR = I2C_SDA_PIN)
#define I2C_SDA_L()         (I2C_SDA_PORT->BRR  = I2C_SDA_PIN)
#define I2C_SDA_READ()      ((I2C_SDA_PORT->IDR & I2C_SDA_PIN) ? 1 : 0)

/* Public API */
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(uint8_t byte);
uint8_t I2C_ReceiveByte(uint8_t ack);
uint8_t I2C_WaitAck(void);
void I2C_Ack(void);
void I2C_NAck(void);

#endif /* __MY_I2C_H */

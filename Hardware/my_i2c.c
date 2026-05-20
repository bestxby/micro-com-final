#include "my_i2c.h"

/* Simple software delay for I2C clock timing (approx 100-200 kHz) */
static void I2C_Delay(void)
{
    volatile uint32_t i = 15;
    while (i--);
}

/**
  * @brief  Initializes the software I2C SCL & SDA pins as Open-Drain.
  * @param  None
  * @retval None
  */
void I2C_Init(void)
{
    /* 1. Enable GPIOB Clock */
    RCC->APB2ENR |= I2C_RCC_ENR;
    (void)RCC->APB2ENR; /* Flush pipeline */

    /* 2. Configure PB6 (SCL) and PB7 (SDA) as Output Open-Drain 50MHz */
    /* CRL bits 24..27 for PB6, 28..31 for PB7. 
       We set CNF=01 (Open-Drain), MODE=11 (Output 50MHz) -> CNF+MODE = 0x7 */
    GPIOB->CRL &= ~0xFF000000;
    GPIOB->CRL |=  0x77000000;

    /* 3. Set both lines High */
    I2C_SCL_H();
    I2C_SDA_H();
    I2C_Delay();
}

/**
  * @brief  Generates I2C Start condition.
  * @param  None
  * @retval None
  */
void I2C_Start(void)
{
    I2C_SDA_H();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SDA_L();
    I2C_Delay();
    I2C_SCL_L();
    I2C_Delay();
}

/**
  * @brief  Generates I2C Stop condition.
  * @param  None
  * @retval None
  */
void I2C_Stop(void)
{
    I2C_SDA_L();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SDA_H();
    I2C_Delay();
}

/**
  * @brief  Sends a byte of data over I2C.
  * @param  byte: The byte to transmit.
  * @retval None
  */
void I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x80) {
            I2C_SDA_H();
        } else {
            I2C_SDA_L();
        }
        I2C_Delay();
        I2C_SCL_H();
        I2C_Delay();
        I2C_SCL_L();
        byte <<= 1;
    }
}

/**
  * @brief  Receives a byte of data over I2C.
  * @param  ack: 1 = Send ACK, 0 = Send NACK.
  * @retval The received byte.
  */
uint8_t I2C_ReceiveByte(uint8_t ack)
{
    uint8_t byte = 0;
    I2C_SDA_H(); /* Release SDA line for input */
    I2C_Delay();
    for (uint8_t i = 0; i < 8; i++) {
        I2C_SCL_H();
        I2C_Delay();
        byte <<= 1;
        if (I2C_SDA_READ()) {
            byte |= 0x01;
        }
        I2C_SCL_L();
        I2C_Delay();
    }
    
    if (ack) {
        I2C_Ack();
    } else {
        I2C_NAck();
    }
    
    return byte;
}

/**
  * @brief  Waits for I2C slave ACK.
  * @param  None
  * @retval 0 = ACK received, 1 = NACK (no ACK) received.
  */
uint8_t I2C_WaitAck(void)
{
    uint8_t ack;
    I2C_SDA_H(); /* Release SDA line for slave to pull low */
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    ack = I2C_SDA_READ();
    I2C_SCL_L();
    I2C_Delay();
    return ack;
}

/**
  * @brief  Generates I2C ACK signal.
  * @param  None
  * @retval None
  */
void I2C_Ack(void)
{
    I2C_SDA_L();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SCL_L();
    I2C_Delay();
}

/**
  * @brief  Generates I2C NACK signal.
  * @param  None
  * @retval None
  */
void I2C_NAck(void)
{
    I2C_SDA_H();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SCL_L();
    I2C_Delay();
}

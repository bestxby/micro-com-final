#include "my_i2c.h"

/* 模拟 I2C 时钟周期简易软件延时 (调谐至约 100-200 kHz) */
static void I2C_Delay(void)
{
    volatile uint32_t i = 15;
    while (i--);
}

/**
  * @brief  初始化软件模拟 I2C 的 SCL 和 SDA GPIO 引脚为开漏输出模式。
  * @param  无
  * @retval 无
  */
void I2C_Init(void)
{
    /* 1. 开启 GPIOB 时钟 */
    RCC->APB2ENR |= I2C_RCC_ENR;
    (void)RCC->APB2ENR; /* 刷新流水线 */

    /* 2. 配置 PB6 (SCL) 和 PB7 (SDA) 为 50MHz 开漏输出模式
       GPIOB->CRL 寄存器中，24..27位控制 PB6，28..31位控制 PB7。
       配置 CNF=01 (开漏输出), MODE=11 (50MHz 输出速度) -> 即 CNF+MODE = 0x7 */
    GPIOB->CRL &= ~0xFF000000;
    GPIOB->CRL |=  0x77000000;

    /* 3. 将信号线默认拉高，置为空闲状态 */
    I2C_SCL_H();
    I2C_SDA_H();
    I2C_Delay();
}

/**
  * @brief  产生 I2C 起始 (Start) 条件。
  * @param  无
  * @retval 无
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
  * @brief  产生 I2C 停止 (Stop) 条件。
  * @param  无
  * @retval 无
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
  * @brief  通过 I2C 物理线发送一个字节数据。
  * @param  byte: 待发送的 8 位字节。
  * @retval 无
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
  * @brief  通过 I2C 物理线接收一个字节数据。
  * @param  ack: 1 = 发送 ACK 应答信号, 0 = 发送 NACK 非应答信号。
  * @retval 接收到的 8 位数据字节。
  */
uint8_t I2C_ReceiveByte(uint8_t ack)
{
    uint8_t byte = 0;
    I2C_SDA_H(); /* 释放 SDA 线为高电平，以便从机控制电平输入 */
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
  * @brief  等待从设备发送的 ACK 应答电平信号。
  * @param  无
  * @retval 0 = 收到 ACK, 1 = 收到 NACK (未收到应答)。
  */
uint8_t I2C_WaitAck(void)
{
    uint8_t ack;
    I2C_SDA_H(); /* 释放 SDA 控制线 */
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    ack = I2C_SDA_READ(); /* 读取物理引脚 */
    I2C_SCL_L();
    I2C_Delay();
    return ack;
}

/**
  * @brief  主机产生 ACK 应答电平信号。
  * @param  无
  * @retval 无
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
  * @brief  主机产生 NACK 非应答电平信号。
  * @param  无
  * @retval 无
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

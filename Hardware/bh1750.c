#include "bh1750.h"
#include "my_i2c.h"

/* 基于标准 72MHz CPU 主频的毫秒级软件延时函数 */
static void BH1750_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/**
  * @brief  初始化 BH1750 传感器并设置为连续高分辨率测量模式。
  * @param  无
  * @retval 0 = 成功, 1 = 失败。
  */
uint8_t BH1750_Init(void)
{
    I2C_Init();
    BH1750_DelayMs(10); /* 芯片上电稳定延时 */

    /* 1. 发送 Power On 指令 (0x01) */
    I2C_Start();
    I2C_SendByte(BH1750_ADDR_WRITE);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }
    I2C_SendByte(0x01); /* Power On */
    I2C_WaitAck();
    I2C_Stop();

    BH1750_DelayMs(5);

    /* 2. 发送连续高分辨率模式测量指令 (0x10) */
    I2C_Start();
    I2C_SendByte(BH1750_ADDR_WRITE);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }
    I2C_SendByte(0x10); /* H-Resolution Mode (1lx resolution) */
    I2C_WaitAck();
    I2C_Stop();

    return 0;
}

/**
  * @brief  读取 BH1750 测量出的光照强度值。
  * @param  lux: 保存勒克斯光照度值的浮点型指针。
  * @retval 0 = 成功, 1 = 失败。
  */
uint8_t BH1750_ReadData(float *lux)
{
    uint8_t h_byte = 0;
    uint8_t l_byte = 0;

    I2C_Start();
    I2C_SendByte(BH1750_ADDR_READ);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }

    h_byte = I2C_ReceiveByte(1); /* 读取高 8 位并发送 ACK */
    l_byte = I2C_ReceiveByte(0); /* 读取低 8 位并发送 NACK，结束传输 */
    I2C_Stop();

    uint16_t raw_val = ((uint16_t)h_byte << 8) | l_byte;
    
    /* 根据数据手册计算 Lux = Raw_Value / 1.2 */
    *lux = (float)raw_val / 1.2f;

    return 0;
}

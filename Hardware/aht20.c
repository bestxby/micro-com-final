#include "aht20.h"
#include "my_i2c.h"

/* 基于标准 72MHz CPU 主频的毫秒级软件延时函数 */
static void AHT20_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/**
  * @brief  读取 AHT20 的状态字节。
  * @param  无
  * @retval 状态字节。
  */
static uint8_t AHT20_ReadStatus(void)
{
    uint8_t status = 0;
    I2C_Start();
    I2C_SendByte(AHT20_ADDR_READ);
    if (I2C_WaitAck() == 0) {
        status = I2C_ReceiveByte(0); /* 发送 NACK 信号 */
    }
    I2C_Stop();
    return status;
}

/**
  * @brief  初始化 AHT20 传感器。
  * @param  无
  * @retval 0 = 初始化成功, 1 = 初始化失败。
  */
uint8_t AHT20_Init(void)
{
    I2C_Init();
    AHT20_DelayMs(40); /* 上电后至少延时 40ms */

    uint8_t status = AHT20_ReadStatus();
    
    /* 如果校准状态位 (bit 3) 为 0，表明未校准，需要发送校准指令 */
    if ((status & 0x08) == 0) {
        I2C_Start();
        I2C_SendByte(AHT20_ADDR_WRITE);
        if (I2C_WaitAck() == 0) {
            I2C_SendByte(0xBE);
            I2C_WaitAck();
            I2C_SendByte(0x08);
            I2C_WaitAck();
            I2C_SendByte(0x00);
            I2C_WaitAck();
        }
        I2C_Stop();
        AHT20_DelayMs(10);
        
        status = AHT20_ReadStatus();
        if ((status & 0x08) == 0) {
            return 1; /* 校准校验失败 */
        }
    }
    
    return 0;
}

/**
  * @brief  从 AHT20 读取温湿度测量数据。
  * @param  temperature: 保存摄氏度温度值的浮点型指针。
  * @param  humidity: 保存相对湿度百分比值的浮点型指针。
  * @retval 0 = 成功, 1 = 失败。
  */
uint8_t AHT20_ReadData(float *temperature, float *humidity)
{
    uint8_t data[6] = {0};
    uint8_t status;
    uint8_t retry = 10;

    /* 1. 发送触发测量命令：0xAC, 0x33, 0x00 */
    I2C_Start();
    I2C_SendByte(AHT20_ADDR_WRITE);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }
    I2C_SendByte(0xAC);
    I2C_WaitAck();
    I2C_SendByte(0x33);
    I2C_WaitAck();
    I2C_SendByte(0x00);
    I2C_WaitAck();
    I2C_Stop();

    /* 2. 延时 80ms 等待传感器内部模数转换完毕 */
    AHT20_DelayMs(80);

    /* 3. 轮询状态字节，等待 Busy 标志位清除 */
    while (retry--) {
        status = AHT20_ReadStatus();
        if ((status & 0x80) == 0) {
            break; /* 空闲，数据已就绪 */
        }
        AHT20_DelayMs(10);
    }
    
    if ((status & 0x80) != 0) {
        return 1; /* 传感器忙状态超时 */
    }

    I2C_Start();
    I2C_SendByte(AHT20_ADDR_READ);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }
    
    data[0] = I2C_ReceiveByte(1); /* 状态字节 */
    data[1] = I2C_ReceiveByte(1); /* 湿度高8位 [19:12] */
    data[2] = I2C_ReceiveByte(1); /* 湿度中8位 [11:4] */
    data[3] = I2C_ReceiveByte(1); /* 湿度低4位 [3:0] 兼 温度高4位 [19:16] */
    data[4] = I2C_ReceiveByte(1); /* 温度中8位 [15:8] */
    data[5] = I2C_ReceiveByte(0); /* 温度低8位 [7:0]，发送 NACK 结束数据传输 */
    I2C_Stop();

    /* 4. 解析原始数据值转换为物理单位 */
    uint32_t raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((data[3] >> 4) & 0x0F);
    uint32_t raw_temp = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *humidity = (float)raw_humidity * 100.0f / 1048576.0f;
    *temperature = (float)raw_temp * 200.0f / 1048576.0f - 50.0f;

    return 0;
}

/**
  * @brief  发送触发测量命令给 AHT20 (非阻塞式第一步)。
  * @retval 0 = 成功, 1 = 失败。
  */
uint8_t AHT20_StartMeasure(void)
{
    I2C_Start();
    I2C_SendByte(AHT20_ADDR_WRITE);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }
    I2C_SendByte(0xAC);
    I2C_WaitAck();
    I2C_SendByte(0x33);
    I2C_WaitAck();
    I2C_SendByte(0x00);
    I2C_WaitAck();
    I2C_Stop();
    return 0;
}

/**
  * @brief  从 AHT20 读取温湿度测量数据 (非阻塞式第二步，需在触发后延时至少 80ms 调用)。
  * @param  temperature: 保存摄氏度温度值的浮点型指针。
  * @param  humidity: 保存相对湿度百分比值的浮点型指针。
  * @retval 0 = 成功, 1 = 失败, 2 = 传感器仍在忙。
  */
uint8_t AHT20_RetrieveData(float *temperature, float *humidity)
{
    uint8_t data[6] = {0};
    uint8_t status = AHT20_ReadStatus();
    
    /* 检查 Busy 标志位 (bit 7) */
    if ((status & 0x80) != 0) {
        return 2; /* 传感器仍在忙，未转换完成 */
    }

    I2C_Start();
    I2C_SendByte(AHT20_ADDR_READ);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }
    
    data[0] = I2C_ReceiveByte(1); /* 状态字节 */
    data[1] = I2C_ReceiveByte(1); /* 湿度高8位 [19:12] */
    data[2] = I2C_ReceiveByte(1); /* 湿度中8位 [11:4] */
    data[3] = I2C_ReceiveByte(1); /* 湿度低4位 [3:0] 兼 温度高4位 [19:16] */
    data[4] = I2C_ReceiveByte(1); /* 温度中8位 [15:8] */
    data[5] = I2C_ReceiveByte(0); /* 温度低8位 [7:0]，发送 NACK 结束数据传输 */
    I2C_Stop();

    /* 解析原始数据值 */
    uint32_t raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((data[3] >> 4) & 0x0F);
    uint32_t raw_temp = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *humidity = (float)raw_humidity * 100.0f / 1048576.0f;
    *temperature = (float)raw_temp * 200.0f / 1048576.0f - 50.0f;

    return 0;
}

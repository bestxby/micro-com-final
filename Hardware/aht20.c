#include "aht20.h"
#include "my_i2c.h"

/* Millisecond delay helper based on standard 72MHz CPU clock */
static void AHT20_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/**
  * @brief  Reads the status byte of AHT20.
  * @param  None
  * @retval Status byte.
  */
static uint8_t AHT20_ReadStatus(void)
{
    uint8_t status = 0;
    I2C_Start();
    I2C_SendByte(AHT20_ADDR_READ);
    if (I2C_WaitAck() == 0) {
        status = I2C_ReceiveByte(0); /* NACK */
    }
    I2C_Stop();
    return status;
}

/**
  * @brief  Initializes the AHT20 sensor.
  * @param  None
  * @retval 0 = Success, 1 = Failure.
  */
uint8_t AHT20_Init(void)
{
    I2C_Init();
    AHT20_DelayMs(40); /* Wait 40ms after power-on */

    uint8_t status = AHT20_ReadStatus();
    
    /* If calibration bit (bit 3) is 0, send calibration command */
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
            return 1; /* Calibration failed */
        }
    }
    
    return 0;
}

/**
  * @brief  Reads temperature and relative humidity from AHT20.
  * @param  temperature: Pointer to float to store temperature in C.
  * @param  humidity: Pointer to float to store relative humidity in %.
  * @retval 0 = Success, 1 = Failure.
  */
uint8_t AHT20_ReadData(float *temperature, float *humidity)
{
    uint8_t data[6] = {0};
    uint8_t status;
    uint8_t retry = 10;

    /* 1. Trigger measurement command: 0xAC, 0x33, 0x00 */
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

    /* 2. Wait 80ms for conversion to complete */
    AHT20_DelayMs(80);

    /* 3. Read status and data bytes */
    while (retry--) {
        status = AHT20_ReadStatus();
        if ((status & 0x80) == 0) {
            break; /* Not busy, data is ready */
        }
        AHT20_DelayMs(10);
    }
    
    if ((status & 0x80) != 0) {
        return 1; /* Sensor busy timeout */
    }

    I2C_Start();
    I2C_SendByte(AHT20_ADDR_READ);
    if (I2C_WaitAck() != 0) {
        I2C_Stop();
        return 1;
    }
    
    data[0] = I2C_ReceiveByte(1); /* Status */
    data[1] = I2C_ReceiveByte(1); /* Humidity [19:12] */
    data[2] = I2C_ReceiveByte(1); /* Humidity [11:4] */
    data[3] = I2C_ReceiveByte(1); /* Humidity [3:0] | Temp [19:16] */
    data[4] = I2C_ReceiveByte(1); /* Temp [15:8] */
    data[5] = I2C_ReceiveByte(0); /* Temp [7:0], NACK */
    I2C_Stop();

    /* 4. Parse raw values */
    uint32_t raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((data[3] >> 4) & 0x0F);
    uint32_t raw_temp = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *humidity = (float)raw_humidity * 100.0f / 1048576.0f;
    *temperature = (float)raw_temp * 200.0f / 1048576.0f - 50.0f;

    return 0;
}

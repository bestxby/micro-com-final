#include "bmp280.h"
#include "my_i2c.h"

/* Calibration Parameters Structure */
typedef struct {
    uint16_t T1;
    int16_t  T2;
    int16_t  T3;
    uint16_t P1;
    int16_t  P2;
    int16_t  P3;
    int16_t  P4;
    int16_t  P5;
    int16_t  P6;
    int16_t  P7;
    int16_t  P8;
    int16_t  P9;
} BMP280_Calib;

static BMP280_Calib calib;
static int32_t t_fine;

/* Software millisecond delay */
static void BMP280_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/* Write to a single BMP280 register */
static void BMP280_WriteReg(uint8_t reg, uint8_t value)
{
    I2C_Start();
    I2C_SendByte(BMP280_ADDR_WRITE);
    I2C_WaitAck();
    I2C_SendByte(reg);
    I2C_WaitAck();
    I2C_SendByte(value);
    I2C_WaitAck();
    I2C_Stop();
}

/* Read a single BMP280 register */
static uint8_t BMP280_ReadReg(uint8_t reg)
{
    uint8_t value = 0;
    I2C_Start();
    I2C_SendByte(BMP280_ADDR_WRITE);
    I2C_WaitAck();
    I2C_SendByte(reg);
    I2C_WaitAck();
    
    I2C_Start(); /* Re-start */
    I2C_SendByte(BMP280_ADDR_READ);
    I2C_WaitAck();
    value = I2C_ReceiveByte(0); /* NACK */
    I2C_Stop();
    return value;
}

/* Read multiple consecutive bytes from BMP280 */
static void BMP280_ReadMulti(uint8_t reg, uint8_t *buf, uint32_t len)
{
    I2C_Start();
    I2C_SendByte(BMP280_ADDR_WRITE);
    I2C_WaitAck();
    I2C_SendByte(reg);
    I2C_WaitAck();
    
    I2C_Start(); /* Re-start */
    I2C_SendByte(BMP280_ADDR_READ);
    I2C_WaitAck();
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = I2C_ReceiveByte(i == (len - 1) ? 0 : 1);
    }
    I2C_Stop();
}

/**
  * @brief  Reads calibration parameters from BMP280 non-volatile memory.
  * @param  None
  * @retval None
  */
static void BMP280_ReadCalib(void)
{
    uint8_t buf[24] = {0};
    BMP280_ReadMulti(BMP280_REG_CALIB, buf, 24);

    calib.T1 = ((uint16_t)buf[1] << 8) | buf[0];
    calib.T2 = ((int16_t)buf[3] << 8)  | buf[2];
    calib.T3 = ((int16_t)buf[5] << 8)  | buf[4];
    
    calib.P1 = ((uint16_t)buf[7] << 8) | buf[6];
    calib.P2 = ((int16_t)buf[9] << 8)  | buf[8];
    calib.P3 = ((int16_t)buf[11] << 8) | buf[10];
    calib.P4 = ((int16_t)buf[13] << 8) | buf[12];
    calib.P5 = ((int16_t)buf[15] << 8) | buf[14];
    calib.P6 = ((int16_t)buf[17] << 8) | buf[16];
    calib.P7 = ((int16_t)buf[19] << 8) | buf[18];
    calib.P8 = ((int16_t)buf[21] << 8) | buf[20];
    calib.P9 = ((int16_t)buf[23] << 8) | buf[22];
}

/**
  * @brief  Initializes the BMP280 sensor.
  * @param  None
  * @retval 0 = Success, 1 = Failure.
  */
uint8_t BMP280_Init(void)
{
    /* 1. Verify Chip ID */
    uint8_t chip_id = BMP280_ReadReg(BMP280_REG_ID);
    if (chip_id != 0x58) {
        return 1; /* Not a BMP280 or connection error */
    }

    /* 2. Soft Reset */
    BMP280_WriteReg(BMP280_REG_RESET, 0xB6);
    BMP280_DelayMs(10);

    /* 3. Read calibration data */
    BMP280_ReadCalib();

    /* 4. Configure sensor
       F4 Ctrl Meas: osrs_t=1 (x1), osrs_p=5 (x16), mode=3 (normal) -> 0x37 
       F5 Config: t_sb=2 (125ms), filter=2 (x4) -> 0x48 */
    BMP280_WriteReg(BMP280_REG_CONFIG, 0x48);
    BMP280_WriteReg(BMP280_REG_CTRL, 0x37);

    return 0;
}

/**
  * @brief  Compensates the raw temperature using calibration coefficients.
  * @param  adc_T: Raw ADC reading of temperature.
  * @retval Temperature in Celsius (float).
  */
static float BMP280_CompensateTemp(int32_t adc_T)
{
    double var1, var2, T;
    var1 = (((double)adc_T) / 16384.0 - ((double)calib.T1) / 1024.0) * ((double)calib.T2);
    var2 = ((((double)adc_T) / 131072.0 - ((double)calib.T1) / 8192.0) *
            (((double)adc_T) / 131072.0 - ((double)calib.T1) / 8192.0)) * ((double)calib.T3);
    t_fine = (int32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0;
    return (float)T;
}

/**
  * @brief  Compensates the raw pressure using calibration coefficients.
  * @param  adc_P: Raw ADC reading of pressure.
  * @retval Pressure in Pascals (float).
  */
static float BMP280_CompensatePress(int32_t adc_P)
{
    double var1, var2, p;
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)calib.P6) / 32768.0;
    var2 = var2 + var1 * ((double)calib.P5) * 2.0;
    var2 = var2 / 4.0 + ((double)calib.P4) * 65536.0;
    var1 = (((double)calib.P3) * var1 * var1 / 524288.0 + ((double)calib.P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)calib.P1);
    if (var1 == 0.0) {
        return 0.0f; /* Avoid division by zero */
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)calib.P9) * p * p / 2147483648.0;
    var2 = p * ((double)calib.P8) / 32768.0;
    p = p + (var1 + var2 + ((double)calib.P7)) / 16.0;
    return (float)p;
}

/**
  * @brief  Reads compensated temperature and pressure from BMP280.
  * @param  temperature: Pointer to float to store temperature in C.
  * @param  pressure: Pointer to float to store pressure in Pa.
  * @retval 0 = Success, 1 = Failure.
  */
uint8_t BMP280_ReadData(float *temperature, float *pressure)
{
    uint8_t buf[6] = {0};
    
    /* Read 6 bytes of ADC data starting from F7 (Pressure MSB) */
    BMP280_ReadMulti(BMP280_REG_PRESS, buf, 6);

    /* Construct raw values */
    int32_t adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);
    int32_t adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)buf[5] >> 4);

    /* Check if default blank values are read (indicates sensor reading failure) */
    if (adc_T == 0x80000 || adc_P == 0x80000) {
        return 1;
    }

    *temperature = BMP280_CompensateTemp(adc_T);
    *pressure = BMP280_CompensatePress(adc_P);

    return 0;
}

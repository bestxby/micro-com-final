#ifndef __BMP280_H
#define __BMP280_H

#include <stdint.h>

/* BMP280 Default I2C Address (SDO grounded) */
#define BMP280_ADDR_WRITE   0xEC  /* 0x76 << 1 */
#define BMP280_ADDR_READ    0xED  /* (0x76 << 1) | 1 */

/* Registers */
#define BMP280_REG_ID       0xD0  /* Chip ID (0x58) */
#define BMP280_REG_RESET    0xE0  /* Soft reset register */
#define BMP280_REG_STATUS   0xF3  /* Status register */
#define BMP280_REG_CTRL     0xF4  /* Control measurement register */
#define BMP280_REG_CONFIG   0xF5  /* Configuration register */
#define BMP280_REG_PRESS    0xF7  /* Pressure MSB start */
#define BMP280_REG_TEMP     0xFA  /* Temperature MSB start */
#define BMP280_REG_CALIB    0x88  /* Calibration data start */

/* Public API */
uint8_t BMP280_Init(void);
uint8_t BMP280_ReadData(float *temperature, float *pressure);

#endif /* __BMP280_H */

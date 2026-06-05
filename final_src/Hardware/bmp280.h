#ifndef __BMP280_H
#define __BMP280_H

#include <stdint.h>

/* BMP280 默认从机 I2C 地址 (SDO引脚接地时) */
#define BMP280_ADDR_WRITE   0xEC  /* 0x76 << 1 */
#define BMP280_ADDR_READ    0xED  /* (0x76 << 1) | 1 */

/* 传感器核心内部寄存器映射 */
#define BMP280_REG_ID       0xD0  /* 芯片特征 ID 寄存器 (读取应为 0x58) */
#define BMP280_REG_RESET    0xE0  /* 软复位寄存器 */
#define BMP280_REG_STATUS   0xF3  /* 运行状态寄存器 */
#define BMP280_REG_CTRL     0xF4  /* 测量参数控制寄存器 */
#define BMP280_REG_CONFIG   0xF5  /* 运行配置寄存器 */
#define BMP280_REG_PRESS    0xF7  /* 气压 ADC 采样高位数据起始寄存器 */
#define BMP280_REG_TEMP     0xFA  /* 温度 ADC 采样高位数据起始寄存器 */
#define BMP280_REG_CALIB    0x88  /* 出厂补偿校准参数存储寄存器起始地址 */

/* 外部公开接口 */
uint8_t BMP280_Init(void);
uint8_t BMP280_ReadData(float *temperature, float *pressure);

#endif /* __BMP280_H */

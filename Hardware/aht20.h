#ifndef __AHT20_H
#define __AHT20_H

#include <stdint.h>

/* AHT20 I2C 8位读写从机地址 */
#define AHT20_ADDR_WRITE    0x70
#define AHT20_ADDR_READ     0x71

/* 外部公开接口 */
uint8_t AHT20_Init(void);
uint8_t AHT20_ReadData(float *temperature, float *humidity);

#endif /* __AHT20_H */

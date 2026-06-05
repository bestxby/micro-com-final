#ifndef __BH1750_H
#define __BH1750_H

#include "stm32f10x.h"

/* I2C Address for BH1750 (ADDR pin pulled low -> 0x23) */
#define BH1750_ADDR_WRITE   0x46   /* (0x23 << 1) */
#define BH1750_ADDR_READ    0x47   /* (0x23 << 1) | 1 */

/* Public API */
uint8_t BH1750_Init(void);
uint8_t BH1750_ReadData(float *lux);

#endif /* __BH1750_H */

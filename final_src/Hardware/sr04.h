#ifndef __SR04_H
#define __SR04_H

#include "stm32f10x.h"

/* Public API */
void SR04_Init(void);
uint8_t SR04_GetDistance(float *dist_cm);

#endif /* __SR04_H */

#ifndef __TM1637_H
#define __TM1637_H

#include "stm32f10x.h"

/* Public API */
void TM1637_Init(void);
void TM1637_DisplayTemp(float temp);
void TM1637_DisplayClear(void);

#endif /* __TM1637_H */

#ifndef __IR_H
#define __IR_H

#include <stdint.h>

// Numeric Pad (0-9, *, #)
#define IR_NUM_1           0x45
#define IR_NUM_2           0x46
#define IR_NUM_3           0x47
#define IR_NUM_4           0x44
#define IR_NUM_5           0x40
#define IR_NUM_6           0x43
#define IR_NUM_7           0x07
#define IR_NUM_8           0x15
#define IR_NUM_9           0x09
#define IR_NUM_0           0x19
#define IR_NUM_STAR        0x16
#define IR_NUM_HASH        0x0D

// Navigation Keys (D-pad below numbers)
#define IR_NAV_UP          0x18
#define IR_NAV_DOWN        0x52
#define IR_NAV_LEFT        0x08
#define IR_NAV_RIGHT       0x5A
#define IR_NAV_OK          0x1C

void IR_Init(void);
uint8_t IR_GetData(uint8_t *code);

#endif /* __IR_H */

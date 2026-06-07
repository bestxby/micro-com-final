#ifndef __IR_H
#define __IR_H

#include <stdint.h>

// Standard NEC keycodes
#define IR_KEY_POWER       0x45
#define IR_KEY_MENU        0x47
#define IR_KEY_MUTE        0x46
#define IR_KEY_MODE        0x44
#define IR_KEY_UP          0x40
#define IR_KEY_DOWN        0x19
#define IR_KEY_LEFT        0x07
#define IR_KEY_RIGHT       0x09
#define IR_KEY_OK          0x15
#define IR_KEY_RETURN      0x43
#define IR_KEY_0           0x16
#define IR_KEY_1           0x0C
#define IR_KEY_2           0x18
#define IR_KEY_3           0x5E
#define IR_KEY_4           0x08
#define IR_KEY_5           0x1C
#define IR_KEY_6           0x5A
#define IR_KEY_7           0x42
#define IR_KEY_8           0x52
#define IR_KEY_9           0x4A

void IR_Init(void);
uint8_t IR_GetData(uint8_t *code);

#endif /* __IR_H */

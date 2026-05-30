#ifndef __GAME_H
#define __GAME_H

#include "stm32f10x.h"

/* 外部公开接口 */
void Game_Init(void);
void Game_Update(uint8_t key_pressed);
void Game_Draw(uint8_t force_refresh);

#endif /* __GAME_H */

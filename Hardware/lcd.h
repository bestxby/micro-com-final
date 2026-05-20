#ifndef __LCD_H
#define __LCD_H

#include "stm32f10x.h"

/* LCD Pin Mapping (4-wire Software SPI) */
#define LCD_RST_PORT        GPIOA
#define LCD_RST_PIN         GPIO_Pin_3
#define LCD_CS_PORT         GPIOA
#define LCD_CS_PIN          GPIO_Pin_4
#define LCD_SCL_PORT        GPIOA
#define LCD_SCL_PIN         GPIO_Pin_5
#define LCD_DC_PORT         GPIOA   /* RS / Data-Command Select */
#define LCD_DC_PIN          GPIO_Pin_6
#define LCD_SDA_PORT        GPIOA   /* MOSI */
#define LCD_SDA_PIN         GPIO_Pin_7
#define LCD_BL_PORT         GPIOB   /* Backlight */
#define LCD_BL_PIN          GPIO_Pin_1

/* Bit-banding physical write macros */
#define LCD_RST_H()         (LCD_RST_PORT->BSRR = LCD_RST_PIN)
#define LCD_RST_L()         (LCD_RST_PORT->BRR  = LCD_RST_PIN)
#define LCD_CS_H()          (LCD_CS_PORT->BSRR  = LCD_CS_PIN)
#define LCD_CS_L()          (LCD_CS_PORT->BRR   = LCD_CS_PIN)
#define LCD_SCL_H()         (LCD_SCL_PORT->BSRR = LCD_SCL_PIN)
#define LCD_SCL_L()         (LCD_SCL_PORT->BRR  = LCD_SCL_PIN)
#define LCD_RS_H()          (LCD_DC_PORT->BSRR  = LCD_DC_PIN)
#define LCD_RS_L()          (LCD_DC_PORT->BRR   = LCD_DC_PIN)
#define LCD_SDA_H()         (LCD_SDA_PORT->BSRR = LCD_SDA_PIN)
#define LCD_SDA_L()         (LCD_SDA_PORT->BRR  = LCD_SDA_PIN)
#define LCD_BL_H()          (LCD_BL_PORT->BSRR  = LCD_BL_PIN)
#define LCD_BL_L()          (LCD_BL_PORT->BRR   = LCD_BL_PIN)

/* Screen Resolution (Default Portrait) */
#define LCD_WIDTH           240
#define LCD_HEIGHT          320

/* RGB565 Color Definitions */
#define BLACK               0x0000
#define WHITE               0xFFFF
#define RED                 0xF800
#define GREEN               0x07E0
#define BLUE                0x001F
#define YELLOW              0xFFE0
#define GRAY                0x8410
#define DARK_GRAY           0x3186
#define CYAN                0x07FF
#define MAGENTA             0xF81F

/* Public API */
void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawRectangle_Filled(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint16_t fc, uint16_t bc);
void LCD_ShowString(uint16_t x, uint16_t y, const char *str, uint16_t fc, uint16_t bc);

#endif /* __LCD_H */

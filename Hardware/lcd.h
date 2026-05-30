#ifndef __LCD_H
#define __LCD_H

#include "stm32f10x.h"

/* ============================================================
 * LCD 引脚配置 (4 线软件模拟 SPI, ST7796S 驱动 IC)
 * ============================================================ */
#define LCD_RST_PORT        GPIOA
#define LCD_RST_PIN         GPIO_Pin_3       /* PA3 → Arduino A3 → FPGA U22 → TFT RST  */
#define LCD_CS_PORT         GPIOA
#define LCD_CS_PIN          GPIO_Pin_4       /* PA4 → Arduino A4 → FPGA V22 → TFT CS   */
#define LCD_SCL_PORT        GPIOA
#define LCD_SCL_PIN         GPIO_Pin_5       /* PA5 → Arduino A5 → FPGA W22 → TFT SCK  */
#define LCD_DC_PORT         GPIOA
#define LCD_DC_PIN          GPIO_Pin_8       /* PA8 → Arduino B3 → FPGA AA22→ TFT DC   */
#define LCD_SDA_PORT        GPIOA
#define LCD_SDA_PIN         GPIO_Pin_7       /* PA7 → Arduino B4 → FPGA V19 → TFT SDI  */
#define LCD_BL_PORT         GPIOB
#define LCD_BL_PIN          GPIO_Pin_1       /* PB1 → Arduino D6 → FPGA Y19 → TFT LED  */

/* 寄存器级位带控制宏 */
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

/* ============================================================
 * 屏幕参数 (ST7796S 驱动, 4.0 inch MSP4020 模组)
 * ============================================================ */
#define LCD_WIDTH           480
#define LCD_HEIGHT          320

/* ============================================================
 * 常用 RGB565 颜色宏
 * ============================================================ */
#define BLACK               0x0000
#define WHITE               0xFFFF
#define RED                 0xF800
#define GREEN               0x07E0
#define BLUE                0x001F
#define YELLOW              0xFFE0
#define CYAN                0x07FF
#define MAGENTA             0xF81F
#define GRAY                0x8410
#define DARK_GRAY           0x3186
#define ORANGE              0xFC00

/* ============================================================
 * 公开 API
 * ============================================================ */
void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/* 像素 & 填充 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/* 几何图形 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
void LCD_FillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

/* 文字 */
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint16_t fc, uint16_t bc);
void LCD_ShowString(uint16_t x, uint16_t y, const char *str, uint16_t fc, uint16_t bc);

#endif /* __LCD_H */

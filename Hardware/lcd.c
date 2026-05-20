#include "lcd.h"
#include "font.h"

/* Software millisecond delay */
static void LCD_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/* Transmit 1 byte via Software SPI */
static void LCD_Writ_Bus(uint8_t dat)
{
    for (uint8_t i = 0; i < 8; i++) {
        LCD_SCL_L();
        if (dat & 0x80) {
            LCD_SDA_H();
        } else {
            LCD_SDA_L();
        }
        LCD_SCL_H();
        dat <<= 1;
    }
}

/* Write LCD command */
static void LCD_WR_REG(uint8_t reg)
{
    LCD_CS_L();
    LCD_RS_L(); /* Command mode (DC low) */
    LCD_Writ_Bus(reg);
    LCD_CS_H();
}

/* Write LCD 8-bit data */
static void LCD_WR_DATA(uint8_t dat)
{
    LCD_CS_L();
    LCD_RS_H(); /* Data mode (DC high) */
    LCD_Writ_Bus(dat);
    LCD_CS_H();
}

/* Write LCD 16-bit data */
static void LCD_WR_DATA16(uint16_t dat)
{
    LCD_CS_L();
    LCD_RS_H(); /* Data mode (DC high) */
    LCD_Writ_Bus(dat >> 8);
    LCD_Writ_Bus(dat & 0xFF);
    LCD_CS_H();
}

/**
  * @brief  Sets the address drawing window on the LCD.
  * @param  x1: Start column coordinate.
  * @param  y1: Start row coordinate.
  * @param  x2: End column coordinate.
  * @param  y2: End row coordinate.
  * @retval None
  */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WR_REG(0x2A); /* Column address set */
    LCD_WR_DATA16(x1);
    LCD_WR_DATA16(x2);
    LCD_WR_REG(0x2B); /* Row address set */
    LCD_WR_DATA16(y1);
    LCD_WR_DATA16(y2);
    LCD_WR_REG(0x2C); /* Memory write command */
}

/**
  * @brief  Initializes the LCD controller (ILI9341/ST7789 compatible) using GPIOs.
  * @param  None
  * @retval None
  */
void LCD_Init(void)
{
    /* 1. Enable Clock for GPIOA and GPIOB */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR; /* Flush pipeline */

    /* 2. Configure PA3..PA7 as 50MHz Push-Pull Outputs (CNF=00, MODE=11 -> 0x3) */
    GPIOA->CRL &= ~0xFFFFF000;
    GPIOA->CRL |=  0x33333000;

    /* 3. Configure PB1 as 50MHz Push-Pull Output (CNF=00, MODE=11 -> 0x3) */
    GPIOB->CRL &= ~0x000000F0;
    GPIOB->CRL |=  0x00000030;

    /* 4. Power on backlight */
    LCD_BL_H();

    /* 5. Hardware Reset Sequence */
    LCD_RST_L();
    LCD_DelayMs(100);
    LCD_RST_H();
    LCD_DelayMs(100);

    /* 6. Send Driver Initialization Sequence */
    LCD_WR_REG(0x11); /* Sleep Out */
    LCD_DelayMs(120);

    LCD_WR_REG(0xCF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0x30);

    LCD_WR_REG(0xED);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x12);
    LCD_WR_DATA(0x81);

    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x78);

    LCD_WR_REG(0xCB);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);

    LCD_WR_REG(0xF7);
    LCD_WR_DATA(0x20);

    LCD_WR_REG(0xEA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC0); /* Power Control 1 */
    LCD_WR_DATA(0x23);

    LCD_WR_REG(0xC1); /* Power Control 2 */
    LCD_WR_DATA(0x10);

    LCD_WR_REG(0xC5); /* VCOM Control 1 */
    LCD_WR_DATA(0x3E);
    LCD_WR_DATA(0x28);

    LCD_WR_REG(0xC7); /* VCOM Control 2 */
    LCD_WR_DATA(0x86);

    LCD_WR_REG(0x36); /* Memory Access Control */
    LCD_WR_DATA(0x08); /* Portrait mode, BGR filter */

    LCD_WR_REG(0x3A); /* Pixel Format */
    LCD_WR_DATA(0x55); /* 16-bit color (RGB565) */

    LCD_WR_REG(0xB1); /* Frame Rate Control */
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x18);

    LCD_WR_REG(0xB6); /* Display Function Control */
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x82);
    LCD_WR_DATA(0x27);

    LCD_WR_REG(0xF2); /* 3Gamma Disable */
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x26); /* Gamma Curve Selected */
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xE0); /* Positive Gamma */
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x31);
    LCD_WR_DATA(0x2B);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x4E);
    LCD_WR_DATA(0xF1);
    LCD_WR_DATA(0x37);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE1); /* Negative Gamma */
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x31);
    LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0x48);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x31);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x0F);

    LCD_WR_REG(0x29); /* Display On */
    LCD_DelayMs(20);
}

/**
  * @brief  Fills the entire screen with a solid color.
  * @param  color: RGB565 color value.
  * @retval None
  */
void LCD_Clear(uint16_t color)
{
    LCD_Address_Set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    LCD_CS_L();
    LCD_RS_H();
    
    uint8_t h = color >> 8;
    uint8_t l = color & 0xFF;
    
    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        /* Write High Byte */
        uint8_t byte = h;
        for (uint8_t b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
        /* Write Low Byte */
        byte = l;
        for (uint8_t b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
    }
    LCD_CS_H();
}

/**
  * @brief  Draws a single pixel on the LCD.
  * @param  x: X coordinate (0..239)
  * @param  y: Y coordinate (0..319)
  * @param  color: RGB565 color value.
  * @retval None
  */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA16(color);
}

/**
  * @brief  Draws a straight line using Bresenham's algorithm.
  * @param  x1, y1: Start coordinate.
  * @param  x2, y2: End coordinate.
  * @param  color: RGB565 color value.
  * @retval None
  */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int16_t dx = (int16_t)x2 - (int16_t)x1;
    int16_t dy = (int16_t)y2 - (int16_t)y1;
    int16_t ux = (dx > 0) ? 1 : -1;
    int16_t uy = (dy > 0) ? 1 : -1;
    dx = (dx > 0) ? dx : -dx;
    dy = (dy > 0) ? dy : -dy;

    uint16_t x = x1, y = y1;
    if (dx > dy) {
        int16_t eps = 0;
        for (x = x1; x != x2 + ux; x += ux) {
            LCD_DrawPoint(x, y, color);
            eps += dy;
            if ((eps << 1) >= dx) {
                y += uy;
                eps -= dx;
            }
        }
    } else {
        int16_t eps = 0;
        for (y = y1; y != y2 + uy; y += uy) {
            LCD_DrawPoint(x, y, color);
            eps += dx;
            if ((eps << 1) >= dy) {
                x += ux;
                eps -= dy;
            }
        }
    }
}

/**
  * @brief  Draws an unfilled rectangle.
  * @param  x1, y1: Top-left corner.
  * @param  x2, y2: Bottom-right corner.
  * @param  color: RGB565 color.
  * @retval None
  */
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

/**
  * @brief  Draws a filled rectangle.
  * @param  x1, y1: Top-left corner.
  * @param  x2, y2: Bottom-right corner.
  * @param  color: RGB565 color.
  * @retval None
  */
void LCD_DrawRectangle_Filled(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_Address_Set(x1, y1, x2, y2);
    LCD_CS_L();
    LCD_RS_H();
    
    uint8_t h = color >> 8;
    uint8_t l = color & 0xFF;
    uint32_t pixels = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);
    
    for (uint32_t i = 0; i < pixels; i++) {
        /* Write High Byte */
        uint8_t byte = h;
        for (uint8_t b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
        /* Write Low Byte */
        byte = l;
        for (uint8_t b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
    }
    LCD_CS_H();
}

/**
  * @brief  Draws a single 8x16 ASCII character.
  * @param  x: X coordinate (0..231)
  * @param  y: Y coordinate (0..304)
  * @param  chr: ASCII character.
  * @param  fc: Font color.
  * @param  bc: Background color.
  * @retval None
  */
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint16_t fc, uint16_t bc)
{
    if (x + 8 > LCD_WIDTH || y + 16 > LCD_HEIGHT) return;
    
    uint8_t temp;
    uint8_t index = chr - ' '; /* Offset by ASCII space */
    
    LCD_Address_Set(x, y, x + 7, y + 15);
    LCD_CS_L();
    LCD_RS_H();
    
    for (uint8_t i = 0; i < 16; i++) {
        temp = asc2_1608[index][i];
        for (uint8_t j = 0; j < 8; j++) {
            uint16_t color = (temp & 0x80) ? fc : bc;
            uint8_t h = color >> 8;
            uint8_t l = color & 0xFF;
            
            /* Write high byte */
            for (uint8_t b = 0; b < 8; b++) {
                LCD_SCL_L();
                if (h & 0x80) LCD_SDA_H(); else LCD_SDA_L();
                LCD_SCL_H();
                h <<= 1;
            }
            /* Write low byte */
            for (uint8_t b = 0; b < 8; b++) {
                LCD_SCL_L();
                if (l & 0x80) LCD_SDA_H(); else LCD_SDA_L();
                LCD_SCL_H();
                l <<= 1;
            }
            temp <<= 1;
        }
    }
    LCD_CS_H();
}

/**
  * @brief  Displays a null-terminated string on the LCD.
  * @param  x: Start X coordinate.
  * @param  y: Start Y coordinate.
  * @param  str: Pointer to string.
  * @param  fc: Font color.
  * @param  bc: Background color.
  * @retval None
  */
void LCD_ShowString(uint16_t x, uint16_t y, const char *str, uint16_t fc, uint16_t bc)
{
    while (*str) {
        if (x + 8 > LCD_WIDTH) {
            x = 0;
            y += 16;
        }
        if (y + 16 > LCD_HEIGHT) break;
        LCD_ShowChar(x, y, *str, fc, bc);
        x += 8;
        str++;
    }
}

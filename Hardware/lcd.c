#include "lcd.h"
#include "font.h"

/* 软件微秒级/毫秒级延时函数 */
static void LCD_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/* 软件模拟 SPI 发送单字节函数 */
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

/* 写 LCD 控制寄存器命令 */
static void LCD_WR_REG(uint8_t reg)
{
    LCD_CS_L();
    LCD_RS_L(); /* 写命令模式 (DC引脚置低) */
    LCD_Writ_Bus(reg);
    LCD_CS_H();
}

/* 写 LCD 8位数据 */
static void LCD_WR_DATA(uint8_t dat)
{
    LCD_CS_L();
    LCD_RS_H(); /* 写数据模式 (DC引脚置高) */
    LCD_Writ_Bus(dat);
    LCD_CS_H();
}

/* 写 LCD 16位数据 */
static void LCD_WR_DATA16(uint16_t dat)
{
    LCD_CS_L();
    LCD_RS_H(); /* 写数据模式 (DC引脚置高) */
    LCD_Writ_Bus(dat >> 8);
    LCD_Writ_Bus(dat & 0xFF);
    LCD_CS_H();
}

/**
  * @brief  设置 LCD 显示的坐标开窗区域。
  * @param  x1: 起始列坐标。
  * @param  y1: 起始行坐标。
  * @param  x2: 结束列坐标。
  * @param  y2: 结束行坐标。
  * @retval 无
  */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WR_REG(0x2A); /* 列地址设置命令 */
    LCD_WR_DATA16(x1);
    LCD_WR_DATA16(x2);
    LCD_WR_REG(0x2B); /* 行地址设置命令 */
    LCD_WR_DATA16(y1);
    LCD_WR_DATA16(y2);
    LCD_WR_REG(0x2C); /* 开始写入显存命令 */
}

/**
  * @brief  使用寄存器配置 LCD 控制引脚，并初始化显示屏控制器 (兼容 ILI9341 与 ST7789)。
  * @param  无
  * @retval 无
  */
void LCD_Init(void)
{
    /* 1. 使能 GPIOA 和 GPIOB 外设时钟 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR; /* 刷新流水线 */

    /* 2. 配置 PA3..PA7 为 50MHz 推挽输出模式 (CNF=00, MODE=11 -> 0x3) */
    GPIOA->CRL &= ~0xFFFFF000;
    GPIOA->CRL |=  0x33333000;

    /* 3. 配置 PB1 为 50MHz 推挽输出模式 (CNF=00, MODE=11 -> 0x3) */
    GPIOB->CRL &= ~0x000000F0;
    GPIOB->CRL |=  0x00000030;

    /* 4. 打开屏幕背光 */
    LCD_BL_H();

    /* 5. 硬件复位脉冲 */
    LCD_RST_L();
    LCD_DelayMs(100);
    LCD_RST_H();
    LCD_DelayMs(100);

    /* 6. 发送控制器初始化指令序列 */
    LCD_WR_REG(0x11); /* 退出休眠模式 (Sleep Out) */
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

    LCD_WR_REG(0xC0); /* 电源控制 1 (Power Control 1) */
    LCD_WR_DATA(0x23);

    LCD_WR_REG(0xC1); /* 电源控制 2 (Power Control 2) */
    LCD_WR_DATA(0x10);

    LCD_WR_REG(0xC5); /* VCOM 控制 1 (VCOM Control 1) */
    LCD_WR_DATA(0x3E);
    LCD_WR_DATA(0x28);

    LCD_WR_REG(0xC7); /* VCOM 控制 2 (VCOM Control 2) */
    LCD_WR_DATA(0x86);

    LCD_WR_REG(0x36); /* 显存访问控制 (Memory Access Control) */
    LCD_WR_DATA(0x08); /* 竖屏显示模式，BGR 滤光片排布 */

    LCD_WR_REG(0x3A); /* 像素格式设置 (Pixel Format) */
    LCD_WR_DATA(0x55); /* 16-bit 像素深度颜色 (RGB565) */

    LCD_WR_REG(0xB1); /* 帧率控制 (Frame Rate Control) */
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x18);

    LCD_WR_REG(0xB6); /* 显示输出控制 (Display Function Control) */
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x82);
    LCD_WR_DATA(0x27);

    LCD_WR_REG(0xF2); /* 启用/关闭 3-Gamma 功能 */
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x26); /* 伽马曲线选择 (Gamma Curve Selected) */
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xE0); /* 正偏置伽马校正 (Positive Gamma) */
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

    LCD_WR_REG(0xE1); /* 负偏置伽马校正 (Negative Gamma) */
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

    LCD_WR_REG(0x29); /* 开启显示输出 (Display On) */
    LCD_DelayMs(20);
}

/**
  * @brief  将整个屏幕清屏填充为指定颜色。
  * @param  color: 16位 RGB565 颜色值。
  * @retval 无
  */
void LCD_Clear(uint16_t color)
{
    LCD_Address_Set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    LCD_CS_L();
    LCD_RS_H();
    
    uint8_t h = color >> 8;
    uint8_t l = color & 0xFF;
    uint8_t b;

    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        /* 单字节快速写入高8位 */
        uint8_t byte = h;
        for (b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
        /* 单字节快速写入低8位 */
        byte = l;
        for (b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
    }
    LCD_CS_H();
}

/**
  * @brief  在 LCD 上画一个像素点。
  * @param  x: X 坐标 (0..239)。
  * @param  y: Y 坐标 (0..319)。
  * @param  color: RGB565 颜色值。
  * @retval 无
  */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA16(color);
}

/**
  * @brief  使用 Bresenham 算法绘制一条直线。
  * @param  x1, y1: 直线起点坐标。
  * @param  x2, y2: 直线终点坐标。
  * @param  color: 直线颜色值。
  * @retval 无
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
  * @brief  绘制空心矩形。
  * @param  x1, y1: 矩形左上角坐标。
  * @param  x2, y2: 矩形右下角坐标。
  * @param  color: 矩形边框颜色。
  * @retval 无
  */
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

/**
  * @brief  绘制填充实心矩形。
  * @param  x1, y1: 矩形左上角坐标。
  * @param  x2, y2: 矩形右下角坐标。
  * @param  color: 填充颜色。
  * @retval 无
  */
void LCD_DrawRectangle_Filled(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_Address_Set(x1, y1, x2, y2);
    LCD_CS_L();
    LCD_RS_H();
    
    uint8_t h = color >> 8;
    uint8_t l = color & 0xFF;
    uint8_t b;
    uint32_t pixels = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);

    for (uint32_t i = 0; i < pixels; i++) {
        /* 单字节快速写入高8位 */
        uint8_t byte = h;
        for (b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
        /* 单字节快速写入低8位 */
        byte = l;
        for (b = 0; b < 8; b++) {
            LCD_SCL_L();
            if (byte & 0x80) LCD_SDA_H(); else LCD_SDA_L();
            LCD_SCL_H();
            byte <<= 1;
        }
    }
    LCD_CS_H();
}

/**
  * @brief  在 LCD 屏幕上绘制单个 8x16 点阵的 ASCII 字符。
  * @param  x: 字符起始 X 坐标。
  * @param  y: 字符起始 Y 坐标。
  * @param  chr: 要显示的字符。
  * @param  fc: 字体颜色。
  * @param  bc: 字符背景色 (用于无闪烁覆盖)。
  * @retval 无
  */
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint16_t fc, uint16_t bc)
{
    if (x + 8 > LCD_WIDTH || y + 16 > LCD_HEIGHT) return;
    
    uint8_t temp;
    uint8_t b;
    uint8_t index = chr - ' '; /* 根据 ASCII 码表偏移计算字模索引 */
    
    LCD_Address_Set(x, y, x + 7, y + 15);
    LCD_CS_L();
    LCD_RS_H();
    
    for (uint8_t i = 0; i < 16; i++) {
        temp = asc2_1608[index][i];
        for (uint8_t j = 0; j < 8; j++) {
            uint16_t color = (temp & 0x80) ? fc : bc;
            uint8_t h = color >> 8;
            uint8_t l = color & 0xFF;
            
            /* 写高8位 */
            for (b = 0; b < 8; b++) {
                LCD_SCL_L();
                if (h & 0x80) LCD_SDA_H(); else LCD_SDA_L();
                LCD_SCL_H();
                h <<= 1;
            }
            /* 写低8位 */
            for (b = 0; b < 8; b++) {
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
  * @brief  在 LCD 上显示以空字符结尾的英文字符串。
  * @param  x: 起始 X 坐标。
  * @param  y: 起始 Y 坐标。
  * @param  str: 字符串指针。
  * @param  fc: 字体颜色。
  * @param  bc: 字符背景色。
  * @retval 无
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

#include "lcd.h"
#include "font.h"

/* ============================================================
 * 内部辅助: 软件延时
 * ============================================================ */
static void LCD_DelayMs(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

/* ============================================================
 * 内部辅助: 软件模拟 SPI 发送单字节
 * ============================================================ */
static void LCD_Writ_Bus(uint8_t dat)
{
    for (uint8_t i = 0; i < 8; i++) {
        LCD_SCL_L();
        if (dat & 0x80) LCD_SDA_H(); else LCD_SDA_L();
        LCD_SCL_H();
        dat <<= 1;
    }
}

/* ============================================================
 * 内部辅助: 写命令 / 写数据 (单字节) / 写数据 (双字节)
 * ============================================================ */
static void LCD_WR_REG(uint8_t reg)
{
    LCD_CS_L();
    LCD_RS_L();
    LCD_Writ_Bus(reg);
    LCD_CS_H();
}

static void LCD_WR_DATA(uint8_t dat)
{
    LCD_CS_L();
    LCD_RS_H();
    LCD_Writ_Bus(dat);
    LCD_CS_H();
}

static void LCD_WR_DATA16(uint16_t dat)
{
    LCD_CS_L();
    LCD_RS_H();
    LCD_Writ_Bus(dat >> 8);
    LCD_Writ_Bus(dat & 0xFF);
    LCD_CS_H();
}

/* ============================================================
 * 内部辅助: 连续写入 16 位像素流 (先发命令 0x2C, 再发数据)
 *           调用前需确保已设置地址窗口且 CS=0, RS=1
 * ============================================================ */
static void LCD_WritePixels(uint16_t color, uint32_t count)
{
    uint8_t h = color >> 8;
    uint8_t l = color & 0xFF;
    while (count--) {
        LCD_Writ_Bus(h);
        LCD_Writ_Bus(l);
    }
}

/* ============================================================
 * 公开 API
 * ============================================================ */

/**
  * @brief  设置显存操作窗口。
  */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WR_REG(0x2A);
    LCD_WR_DATA16(x1);
    LCD_WR_DATA16(x2);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA16(y1);
    LCD_WR_DATA16(y2);
    LCD_WR_REG(0x2C);
}

/**
  * @brief  初始化 ST7796S 控制器 (320×480 竖屏, RGB565, 4 线 SPI)。
  */
void LCD_Init(void)
{
    /* ---- 1. 时钟 ---- */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR;

    /* ---- 2. PA3, PA4, PA5, PA7: 50MHz 推挽输出 (排除用作 I2C SCL 的 PA6) ---- */
    GPIOA->CRL &= ~0xF0FFF000;
    GPIOA->CRL |=  0x30333000;

    /* ---- 3. PA8 (LCD_DC): 50MHz 推挽输出 ---- */
    GPIOA->CRH &= ~0x0000000F;
    GPIOA->CRH |=  0x00000003;

    /* ---- 4. PB1 (LCD_BL): 50MHz 推挽输出 ---- */
    GPIOB->CRL &= ~0x000000F0;
    GPIOB->CRL |=  0x00000030;

    /* ---- 5. 背光 & 复位 ---- */
    LCD_BL_H();
    LCD_RST_L();  LCD_DelayMs(100);
    LCD_RST_H();  LCD_DelayMs(100);

    /* ---- 6. ST7796S 初始化序列 ---- */
    LCD_WR_REG(0x11); LCD_DelayMs(120);   /* Sleep Out */

    LCD_WR_REG(0x36);
    LCD_WR_DATA(LCD_MADCTL_VALUE);         /* MADCTL: 采用 lcd.h 中配置的屏幕方向 */

    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);                     /* 16-bit RGB565 */

    /* 电源设置 */
    LCD_WR_REG(0xC0); LCD_WR_DATA(0x23);   /* Power Control 1 */
    LCD_WR_REG(0xC1); LCD_WR_DATA(0x10);   /* Power Control 2 */
    LCD_WR_REG(0xC5); LCD_WR_DATA(0x3E); LCD_WR_DATA(0x28); /* VCOM */
    LCD_WR_REG(0xC7); LCD_WR_DATA(0x86);   /* VCOM Control 2 */

    /* 帧率 */
    LCD_WR_REG(0xB1); LCD_WR_DATA(0x00); LCD_WR_DATA(0x18);
    LCD_WR_REG(0xB6); LCD_WR_DATA(0x08); LCD_WR_DATA(0x82); LCD_WR_DATA(0x3B); /* 0x3B = 59, (59+1)*8 = 480 行驱动 */

    /* Gamma */
    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0x0F); LCD_WR_DATA(0x31); LCD_WR_DATA(0x2B); LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x0E); LCD_WR_DATA(0x08); LCD_WR_DATA(0x4E); LCD_WR_DATA(0xF1);
    LCD_WR_DATA(0x37); LCD_WR_DATA(0x07); LCD_WR_DATA(0x10); LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x0E); LCD_WR_DATA(0x09); LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0x00); LCD_WR_DATA(0x0E); LCD_WR_DATA(0x14); LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x11); LCD_WR_DATA(0x07); LCD_WR_DATA(0x31); LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0x48); LCD_WR_DATA(0x08); LCD_WR_DATA(0x0F); LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x31); LCD_WR_DATA(0x36); LCD_WR_DATA(0x0F);

    /* 3-Gamma / Gamma 曲线 */
    LCD_WR_REG(0xF2); LCD_WR_DATA(0x00);
    LCD_WR_REG(0x26); LCD_WR_DATA(0x01);

    /* 其他 */
    LCD_WR_REG(0xCF); LCD_WR_DATA(0x00); LCD_WR_DATA(0xC1); LCD_WR_DATA(0x30);
    LCD_WR_REG(0xED); LCD_WR_DATA(0x64); LCD_WR_DATA(0x03); LCD_WR_DATA(0x12); LCD_WR_DATA(0x81);
    LCD_WR_REG(0xE8); LCD_WR_DATA(0x85); LCD_WR_DATA(0x00); LCD_WR_DATA(0x78);
    LCD_WR_REG(0xCB); LCD_WR_DATA(0x39); LCD_WR_DATA(0x2C); LCD_WR_DATA(0x00); LCD_WR_DATA(0x34); LCD_WR_DATA(0x02);
    LCD_WR_REG(0xF7); LCD_WR_DATA(0x20);
    LCD_WR_REG(0xEA); LCD_WR_DATA(0x00); LCD_WR_DATA(0x00);

    LCD_WR_REG(0x29); LCD_DelayMs(20);     /* Display On */
}

/**
  * @brief  动态设置屏幕方向与扫描模式。
  */
void LCD_SetOrientation(uint8_t madctl)
{
    LCD_WR_REG(0x36);
    LCD_WR_DATA(madctl);
}


/**
  * @brief  全屏填充纯色 (优化: 单次开窗 + 连续数据流)。
  */
void LCD_Clear(uint16_t color)
{
    LCD_Address_Set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    LCD_CS_L();
    LCD_RS_H();
    LCD_WritePixels(color, (uint32_t)LCD_WIDTH * LCD_HEIGHT);
    LCD_CS_H();
}

/**
  * @brief  画点。
  */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA16(color);
}

/**
  * @brief  填充矩形区域 (优化: 单次开窗 + 连续数据流)。
  */
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH  - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    LCD_Address_Set(x, y, x + w - 1, y + h - 1);
    LCD_CS_L();
    LCD_RS_H();
    LCD_WritePixels(color, (uint32_t)w * h);
    LCD_CS_H();
}

/* ---- 几何绘图 ---- */

/**
  * @brief  Bresenham 直线。
  */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int16_t dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int16_t dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int16_t sx = (x1 < x2) ? 1 : -1;
    int16_t sy = (y1 < y2) ? 1 : -1;
    int16_t err = dx - dy, e2;
    while (1) {
        LCD_DrawPoint(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = err << 1;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

/**
  * @brief  空心矩形。
  */
void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    LCD_DrawLine(x, y, x + w - 1, y, color);
    LCD_DrawLine(x, y, x, y + h - 1, color);
    LCD_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    LCD_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
}

/**
  * @brief  空心圆。
  */
void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a = 0, b = r, di = 3 - (r << 1);
    while (a <= b) {
        LCD_DrawPoint(x0 + a, y0 + b, color); LCD_DrawPoint(x0 - a, y0 + b, color);
        LCD_DrawPoint(x0 + a, y0 - b, color); LCD_DrawPoint(x0 - a, y0 - b, color);
        LCD_DrawPoint(x0 + b, y0 + a, color); LCD_DrawPoint(x0 - b, y0 + a, color);
        LCD_DrawPoint(x0 + b, y0 - a, color); LCD_DrawPoint(x0 - b, y0 - a, color);
        a++;
        if (di < 0) { di += (a << 2) + 6; }
        else        { di += ((a - b) << 2) + 10; b--; }
    }
}

/**
  * @brief  实心圆。
  */
void LCD_FillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a = 0, b = r, di = 3 - (r << 1);
    while (a <= b) {
        LCD_DrawLine(x0 - a, y0 + b, x0 + a, y0 + b, color);
        LCD_DrawLine(x0 - a, y0 - b, x0 + a, y0 - b, color);
        LCD_DrawLine(x0 - b, y0 + a, x0 + b, y0 + a, color);
        LCD_DrawLine(x0 - b, y0 - a, x0 + b, y0 - a, color);
        a++;
        if (di < 0) { di += (a << 2) + 6; }
        else        { di += ((a - b) << 2) + 10; b--; }
    }
}

/* ---- 文字显示 ---- */

/**
  * @brief  显示 8×16 ASCII 字符 (带背景色)。
  */
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint16_t fc, uint16_t bc)
{
    if (x + 8 > LCD_WIDTH || y + 16 > LCD_HEIGHT) return;
    uint8_t index = chr - ' ';
    LCD_Address_Set(x, y, x + 7, y + 15);
    LCD_CS_L();
    LCD_RS_H();
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t row = asc2_1608[index][i];
        for (uint8_t j = 0; j < 8; j++) {
            uint16_t color = (row & 0x80) ? fc : bc;
            LCD_Writ_Bus(color >> 8);
            LCD_Writ_Bus(color & 0xFF);
            row <<= 1;
        }
    }
    LCD_CS_H();
}

/**
  * @brief  显示字符串 (自动换行, 超出裁剪)。
  */
void LCD_ShowString(uint16_t x, uint16_t y, const char *str, uint16_t fc, uint16_t bc)
{
    uint16_t cx = x;
    while (*str) {
        if (*str == '\n') { cx = x; y += 16; str++; continue; }
        if (cx + 8 > LCD_WIDTH)  { cx = x; y += 16; }
        if (y + 16 > LCD_HEIGHT) break;
        LCD_ShowChar(cx, y, *str, fc, bc);
        cx += 8; str++;
    }
}

/* ============================================================
 * 动态主题配置实现
 * ============================================================ */
uint8_t current_theme = 0; // 0 = Dark (Neon), 1 = Light (Claude Warm Paper)
uint16_t theme_bg = BLACK;
uint16_t theme_card_bg = DARK_GRAY;
uint16_t theme_text = WHITE;
uint16_t theme_text_muted = GRAY;
uint16_t theme_border = GRAY;
uint16_t theme_accent = CYAN;
uint16_t theme_green = GREEN;
uint16_t theme_blue = BLUE;
uint16_t theme_red = RED;
uint16_t theme_yellow = YELLOW;

void Theme_Apply(void)
{
    if (current_theme == 0) {
        // Dark Theme (Neon style)
        theme_bg = BLACK;
        theme_card_bg = DARK_GRAY;
        theme_text = WHITE;
        theme_text_muted = GRAY;
        theme_border = GRAY;
        theme_accent = CYAN;
        theme_green = GREEN;
        theme_blue = BLUE;
        theme_red = RED;
        theme_yellow = YELLOW;
    } else {
        // Light Theme (Claude warm paper style)
        theme_bg = 0xF7BD;        // Warm cream page bg (#F7F4EB)
        theme_card_bg = 0xFFFF;   // Clean white card bg (#FFFFFF)
        theme_text = 0x18C3;      // Charcoal brown text (#1E1B18)
        theme_text_muted = 0x7BEF;// Muted gray-brown text (#7E7B78)
        theme_border = 0xD6BA;    // Soft warm gray border (#D6D3C9)
        theme_accent = 0xDB48;    // Warm terracotta/orange accent (#D96B43)
        theme_green = 0x24C8;     // Soft green (#209840)
        theme_blue = 0x1B35;      // Deep soft blue (#1F619F)
        theme_red = 0xC104;       // Crimson red (#C02020)
        theme_yellow = 0xB440;    // Ochre/amber yellow (#B48800)
    }
}

void Theme_Init(void)
{
    Theme_Apply();
}


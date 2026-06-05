#include "tm1637.h"

/* Register Bit Banding / Direct I/O macros for PB13 and PB14 */
#define TM_CLK_H()     (GPIOB->BSRR = GPIO_Pin_13)
#define TM_CLK_L()     (GPIOB->BRR  = GPIO_Pin_13)
#define TM_DIO_H()     (GPIOB->BSRR = GPIO_Pin_14)
#define TM_DIO_L()     (GPIOB->BRR  = GPIO_Pin_14)
#define TM_DIO_READ()  ((GPIOB->IDR & GPIO_Pin_14) ? 1 : 0)

static const uint8_t tm1637_digit_map[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

/* Software delay helper */
static void TM1637_DelayUs(uint32_t us)
{
    volatile uint32_t count = us * 10;
    while (count--);
}

/* Bus start condition */
static void TM1637_Start(void)
{
    TM_CLK_H();
    TM_DIO_H();
    TM1637_DelayUs(5);
    TM_DIO_L();
    TM1637_DelayUs(5);
    TM_CLK_L();
}

/* Bus stop condition */
static void TM1637_Stop(void)
{
    TM_CLK_L();
    TM_DIO_L();
    TM1637_DelayUs(5);
    TM_CLK_H();
    TM1637_DelayUs(5);
    TM_DIO_H();
    TM1637_DelayUs(5);
}

/* Send 1 byte and read ACK */
static uint8_t TM1637_WriteByte(uint8_t dat)
{
    for (uint8_t i = 0; i < 8; i++) {
        TM_CLK_L();
        if (dat & 0x01) {
            TM_DIO_H();
        } else {
            TM_DIO_L();
        }
        TM1637_DelayUs(5);
        TM_CLK_H();
        TM1637_DelayUs(5);
        dat >>= 1;
    }
    
    /* ACK check */
    TM_CLK_L();
    TM_DIO_H(); /* Pull up to allow slave to pull low */
    TM1637_DelayUs(5);
    TM_CLK_H();
    TM1637_DelayUs(5);
    uint8_t ack = TM_DIO_READ();
    TM_CLK_L();
    
    return ack;
}

/* Write to 4-digit display registers */
static void TM1637_Display(uint8_t *data)
{
    /* Data Command: Automatic address addition, normal mode */
    TM1637_Start();
    TM1637_WriteByte(0x40);
    TM1637_Stop();
    
    /* Address Command: Start at C0 */
    TM1637_Start();
    TM1637_WriteByte(0xC0);
    for (uint8_t i = 0; i < 4; i++) {
        TM1637_WriteByte(data[i]);
    }
    TM1637_Stop();
    
    /* Control Command: Display ON, brightness level 3 (mid) */
    TM1637_Start();
    TM1637_WriteByte(0x8B);
    TM1637_Stop();
}

/* Public APIs */

void TM1637_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR;
    
    /* Configure PB13 (CLK) as push-pull output, PB14 (DIO) as open-drain output */
    GPIOB->CRH &= ~0x0FF00000;
    GPIOB->CRH |=  0x07300000;
    
    TM_CLK_H();
    TM_DIO_H();
    
    TM1637_DisplayClear();
}

void TM1637_DisplayTemp(float temp)
{
    uint8_t data[4] = {0};
    uint8_t is_neg = 0;
    
    if (temp < 0) {
        is_neg = 1;
        temp = -temp;
    }
    
    if (temp > 99.9f) {
        /* High temperature out-of-range display (99.9C) */
        data[0] = tm1637_digit_map[9];
        data[1] = tm1637_digit_map[9] | 0x80;
        data[2] = tm1637_digit_map[9];
        data[3] = 0x39; /* 'C' */
    } else {
        int temp_val = (int)(temp * 10.0f + 0.5f);
        int d2 = temp_val % 10;
        int d1 = (temp_val / 10) % 10;
        int d0 = (temp_val / 100) % 10;
        
        if (is_neg) {
            data[0] = 0x40; /* Minus sign */
            data[1] = tm1637_digit_map[d1] | 0x80;
            data[2] = tm1637_digit_map[d2];
        } else {
            if (d0 == 0) {
                data[0] = 0x00; /* Blank leading zero */
            } else {
                data[0] = tm1637_digit_map[d0];
            }
            data[1] = tm1637_digit_map[d1] | 0x80;
            data[2] = tm1637_digit_map[d2];
        }
        data[3] = 0x39; /* 'C' */
    }
    
    TM1637_Display(data);
}

void TM1637_DisplayClear(void)
{
    uint8_t data[4] = {0, 0, 0, 0};
    TM1637_Display(data);
}

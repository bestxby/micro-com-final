#include "touch.h" 
#include "lcd.h"
#include "stdlib.h"
#include "math.h"

// Define local structures/variables to compile without QDtech external libraries

struct {
    uint16_t width;
    uint16_t height;
} lcddev = {480, 320};

// Mock/Adapter for circle drawing using LCD driver functions
static void gui_circle(uint16_t x, uint16_t y, uint16_t color, uint16_t r, uint8_t fill)
{
    if (fill) {
        LCD_FillCircle(x, y, r, color);
    } else {
        LCD_DrawCircle(x, y, r, color);
    }
}

// Software delays
void delay_us(uint32_t us)
{
    volatile uint32_t count = us * 7;
    while (count--);
}

void delay_ms(uint32_t ms)
{
    volatile uint32_t count = ms * 7200;
    while (count--);
}

_m_tp_dev tp_dev =
{
	TP_Init,
	TP_Scan,
	TP_Adjust,
	0,
	0,
 	0,
	0,
	0,
	0.135f,   // Default xfac
	0.090f,   // Default yfac
	-40,      // Default xoff
	-20,      // Default yoff
	0,        // Default touchtype
};					

uint8_t CMD_RDX = 0xD0;
uint8_t CMD_RDY = 0x90;

void TP_Write_Byte(uint8_t num)    
{  
	uint8_t count = 0; 
	TCLK = 0; 
	TCS = 0;	
	for (count = 0; count < 8; count++)  
	{ 	  
		if (num & 0x80) TDIN = 1;  
		else TDIN = 0; 
		num <<= 1;    
		TCLK = 0; 
		delay_us(1);		
		TCLK = 1;		// 上升沿有效
		delay_us(1);		
	}		 			    
}

uint16_t TP_Read_AD(uint8_t CMD)	  
{ 	 
	uint8_t count = 0; 	  
	uint16_t Num = 0; 
	TCLK = 0;		// 先拉低时钟 	 
	TDIN = 0; 	// 拉低数据线
	TCS = 0; 		// 选中触摸屏IC
	TP_Write_Byte(CMD); // 发送命令字
		
	delay_us(6); // XPT2046的转换时间最长为6us		     	      	   
	  
	for (count = 0; count < 12; count++) // 读出12位数据
	{ 				  
		Num <<= 1; 
		TCLK = 1;		 
		delay_us(1);  
		TCLK = 0; 	
		delay_us(1);
			
		if (DOUT) Num |= 1;
	} 

	TCS = 1;		// 释放片选	 
	TCLK = 0;		// 先拉低时钟 	 
	TDIN = 0; 	// 拉低数据线
	
	return Num;  
}

#define READ_TIMES 5 	// 读取次数
#define LOST_VAL 1	  	// 丢弃值

uint16_t TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum = 0;
	uint16_t temp;
	for (i = 0; i < READ_TIMES; i++) { buf[i] = TP_Read_AD(xy); delay_us(2); } 		    
	for (i = 0; i < READ_TIMES - 1; i++) // 排序
	{
		for (j = i + 1; j < READ_TIMES; j++)
		{
			if (buf[i] > buf[j]) // 升序排列
			{
				temp = buf[i];
				buf[i] = buf[j];
				buf[j] = temp;
			}
		}
	}	  
	sum = 0;
	for (i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++) sum += buf[i];
	temp = sum / (READ_TIMES - 2 * LOST_VAL);
	return temp;   
} 

uint8_t TP_Read_XY(uint16_t *x, uint16_t *y)
{
	uint16_t xtemp, ytemp;			 	 		  
	xtemp = TP_Read_XOY(CMD_RDX);
	ytemp = TP_Read_XOY(CMD_RDY);	
	
	*x = xtemp;
	*y = ytemp;
	
	return 1; // 读数成功
}

#define ERR_RANGE 50 // 误差范围 

uint8_t TP_Read_XY2(uint16_t *x, uint16_t *y) 
{
	uint16_t x1, y1;
 	uint16_t x2, y2;
 	uint8_t flag;    
	flag = TP_Read_XY(&x1, &y1);   
	if (flag == 0) return 0;
	flag = TP_Read_XY(&x2, &y2);	   
	if (flag == 0) return 0;   
	if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) // 前后两次采样在+-50内
	&& ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
	{
		*x = (x1 + x2) / 2;
		*y = (y1 + y2) / 2;
		return 1;
	} else return 0;	  
} 

void TP_Drow_Touch_Point(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_DrawLine(x - 12, y, x + 13, y, color); // 横线
	LCD_DrawLine(x, y - 12, x, y + 13, color); // 竖线
	LCD_DrawPoint(x + 1, y + 1, color);
	LCD_DrawPoint(x - 1, y + 1, color);
	LCD_DrawPoint(x + 1, y - 1, color);
	LCD_DrawPoint(x - 1, y - 1, color);
	gui_circle(x, y, color, 6, 0); // 画中心圈
}	

void TP_Draw_Big_Point(uint16_t x, uint16_t y, uint16_t color)
{	    
	LCD_DrawPoint(x, y, color); // 中心点 
	LCD_DrawPoint(x + 1, y, color);
	LCD_DrawPoint(x, y + 1, color);
	LCD_DrawPoint(x + 1, y + 1, color);	 	  	
}	

uint8_t TP_Scan(uint8_t tp)
{			   
	if (PEN == 0) // 有触摸被按下
	{
		if (tp) { 
			TP_Read_XY2(&tp_dev.x, &tp_dev.y); // 读取物理坐标
		} else if (TP_Read_XY2(&tp_dev.x, &tp_dev.y)) { // 读取并转换屏幕坐标
	 		tp_dev.x = tp_dev.xfac * tp_dev.x + tp_dev.xoff;
			tp_dev.y = tp_dev.yfac * tp_dev.y + tp_dev.yoff;  
	 	} 
		if ((tp_dev.sta & TP_PRES_DOWN) == 0) // 之前没有被按下
		{		 
			tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES; // 标记按键按下  
			tp_dev.x0 = tp_dev.x; // 记录第一次按下时的坐标
			tp_dev.y0 = tp_dev.y;  	  
		}			   
	} else {
		if (tp_dev.sta & TP_PRES_DOWN) // 之前是被按下的
		{
			tp_dev.sta &= ~(1 << 7); // 标记按键松开	
		} else {
			tp_dev.x0 = 0;
			tp_dev.y0 = 0;
			tp_dev.x = 0xffff;
			tp_dev.y = 0xffff;
		}	    
	}
	return tp_dev.sta & TP_PRES_DOWN; // 返回当前的触屏状态
}
	  
void TP_Save_Adjdata(void)
{
	// 默认使用硬编码数据，不需要写Flash
}

uint8_t TP_Get_Adjdata(void)
{					  
	tp_dev.xfac = 0.135f;
	tp_dev.xoff = -40;
	tp_dev.yfac = 0.090f;
	tp_dev.yoff = -20;
	tp_dev.touchtype = 0;
	
	CMD_RDX = 0XD0;
	CMD_RDY = 0X90;
	return 1;
}	
 
void TP_Adj_Info_Show(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t fac)
{	  
	// 无需实现
}

void TP_Adjust(void)
{								 
	// 无需实现
}		

uint8_t TP_Init(void)
{			    		   
	// 使能GPIOA、GPIOB和AFIO时钟
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
	(void)RCC->APB2ENR; // 强刷时钟总线流水线
	
	// 解除PB3、PB4 JTAG复用，使能SWD并将其作为普通IO使用
	AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG) | AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

	// 配置GPIOB的Pin3(T_DIN)、Pin4(T_CS)、Pin5(T_CLK)为推挽输出 (50MHz, CNF=00, MODE=11 -> 0x3)
	GPIOB->CRL = (GPIOB->CRL & ~0x00FFF000) | 0x00333000;

	// 配置GPIOA的Pin11(T_IRQ/PEN)和Pin12(T_DO/DOUT)为上拉输入 (CNF=10, MODE=00 -> 0x8)
	GPIOA->CRH = (GPIOA->CRH & ~0x000FF000) | 0x00088000;
	GPIOA->ODR |= (1 << 11) | (1 << 12); // 开启上拉电阻
  
	TCS = 1;
	TCLK = 0;
	
	TP_Read_XY(&tp_dev.x, &tp_dev.y); // 第一次读取初始化	 
	TP_Get_Adjdata();	
	
	return 1; 									 
}

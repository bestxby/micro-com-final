`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Module Name: EES351_TOP
// Description: EES-351-MC 实验箱 FPGA 顶层桥接模块
//              将 STM32 (Arduino/PMOD 排针) 信号路由至底板各外设
//
// 引脚依据: u5 工程源码 (lcd.h / led.h / key.h / my_i2c.h)
//           + 用户提供的 Arduino↔STM32 映射
//           + EES-351-MC 硬件手册
//////////////////////////////////////////////////////////////////////////////////


module EES351_TOP(
    // ==================== Group A: TFT LCD SPI 接口 ====================
    // Arduino 侧输入 (来自 STM32)
    input      A3_LCD_RST ,    // 硬件复位 : STM32 PA3 → Arduino A3 → U22
    input      A4_LCD_CS  ,    // SPI 片选  : STM32 PA4 → Arduino A4 → V22
    input      A5_LCD_SCK ,    // SPI 时钟  : STM32 PA5 → Arduino A5 → W22
    input      B4_LCD_SDA ,    // SPI 数据  : STM32 PA7 → Arduino B4 → V19
    input      D6_LCD_BL  ,    // 背光控制  : STM32 PB1 → Arduino D6 → Y19
    input      B3_LCD_DC  ,    // 数据/命令 : STM32 PA8 → Arduino B3 → AA22

    // TFT 侧输出 → J5 连接器
    output     TFT_RST     ,    // → N20  TFT 硬件复位
    output     TFT_CS      ,    // → N19  TFT 片选
    output     TFT_SCK     ,    // → N22  TFT 时钟
    output     TFT_SDI     ,    // → M22  TFT MOSI 数据
    output     TFT_DC      ,    // → M21  TFT 数据/命令选择
    output     TFT_LED     ,    // → P22  TFT 背光

    // ==================== Group B: LED 指示灯 ====================
    // Arduino 侧输入 (来自 STM32)
    input      A0_LED2     ,    // LED2 呼吸灯 : STM32 PA0  → Arduino A0 → T21
    input      A1_LED3     ,    // LED3 预留   : STM32 PA1  → Arduino A1 → U21
    input      A2_LED4     ,    // LED4 预留   : STM32 PA2  → Arduino A2 → T22
    input      B5_LED1     ,    // LED1 告警灯 : STM32 PC13 → Arduino B5 → V18

    // 底板 LED 输出
    output     BASE_LED0   ,    // → U16  底板 LED0
    output     BASE_LED1   ,    // → U17  底板 LED1
    output     BASE_LED2   ,    // → U15  底板 LED2
    output     BASE_LED3   ,    // → V15  底板 LED3

    // ==================== Group C: 独立按键 ====================
    // 底板按键输入
    input      BASE_BTN1   ,    // ← H15  底板按键 PB17 (S0)
    input      BASE_BTN2   ,    // ← K15  底板按键 PB18 (S1)
    input      BASE_BTN3   ,    // ← J15  底板按键 PB19 (S2)
    input      BASE_BTN4   ,    // ← W18  底板按键 PB20 (S3)

    // Arduino 侧输出 (→ STM32)
    output     D7_KEY1     ,    // KEY1 页面切换 : → Arduino D7 → STM32 PB0
    output     D0_KEY2     ,    // KEY2 系统复位 : → Arduino D0 → STM32 PB8
    output     B0_OUT      ,    // 左键 S2 : → Arduino B0 → STM32 PA15

    // ==================== Group D: I2C 总线 (直连底板, 不经 FPGA) ====================
    // I2C_SCL : STM32 PB6  ↔ Arduino D2 ↔ AB19  (FPGA 预留)
    // I2C_SDA : STM32 PB10 (非Arduino, 已释放 D1/AB20 给 Touch 备用)

    // ==================== Group E: 触摸屏 SPI + IRQ ====================
    // Arduino 侧 — STM32 → FPGA (OUTPUT from STM32, FPGA INPUT)
    input      D4_TCS     ,    // 触摸 CS   : STM32 PB4  → Arduino D4 → AA19
    input      D5_TDIN    ,    // 触摸 MOSI : STM32 PB3  → Arduino D5 → W17
    input      D3_TCLK    ,    // 触摸 SCLK : STM32 PB5  → Arduino D3 → Y21

    // Arduino 侧 — FPGA → STM32 (FPGA OUTPUT to STM32 INPUT)
    output     B2_PEN     ,    // 触摸 IRQ  : → Arduino B2 → STM32 PA11
    output     B1_DOUT    ,    // 触摸 MISO : → Arduino B1 → STM32 PA12

    // TFT 触摸面板侧 (→ J5 连接器)
    output     TOUCH_CS   ,    // → R21  T_CS  (J5-15)
    output     TOUCH_DIN  ,    // → P20  T_DIN (J5-16)
    output     TOUCH_CLK  ,    // → M19  T_CLK (J5-12)
    input      TOUCH_IRQ  ,    // ← P15  T_IRQ (J5-16)
    input      TOUCH_DO        // ← P21  T_DO  (J5-16)
);


// ==================== Group A: LCD 信号透传 ====================
assign TFT_RST  = A3_LCD_RST ;
assign TFT_CS   = A4_LCD_CS  ;
assign TFT_SCK  = A5_LCD_SCK ;
assign TFT_SDI  = B4_LCD_SDA ;
assign TFT_DC   = B3_LCD_DC  ;
assign TFT_LED  = D6_LCD_BL  ;


// ==================== Group B: LED 信号透传 ====================
assign BASE_LED0 = A0_LED2 ;
assign BASE_LED1 = A1_LED3 ;
assign BASE_LED2 = A2_LED4 ;
assign BASE_LED3 = B5_LED1 ;


// ==================== Group C: 按键信号透传 ====================
assign D7_KEY1 = BASE_BTN1 ;
assign D0_KEY2 = BASE_BTN2 ;
assign B0_OUT  = BASE_BTN3 ; // Re-route Left Button S2 to Arduino B0 (STM32 PA15)


// ==================== Group E: 触摸屏信号透传 (5 路全部启用) ====================
assign TOUCH_CS  = D4_TCS  ;
assign TOUCH_DIN = D5_TDIN ;
assign TOUCH_CLK = D3_TCLK ;
assign B2_PEN    = TOUCH_IRQ ; // Restore TOUCH_IRQ to Arduino B2 (STM32 PA11)
assign B1_DOUT   = BASE_BTN4 ; // Re-route Right Button S3 to Arduino B1 (STM32 PA12)


endmodule

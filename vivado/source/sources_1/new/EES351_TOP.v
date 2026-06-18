`timescale 1ns / 1ps
module EES351_TOP(
    // ==================== Group A: TFT LCD SPI 接口 ====================
    output     STM_UART_TX,    // 串口发往 STM32 PA3 (原 A3_LCD_RST → U22)
    input      A4_LCD_CS  ,    
    input      A5_LCD_SCK ,    
    input      B4_LCD_SDA ,    
    input      D6_LCD_BL  ,    
    input      B3_LCD_DC  ,    

    output     TFT_RST     ,    
    output     TFT_CS      ,    
    output     TFT_SCK     ,    
    output     TFT_SDI     ,    
    output     TFT_DC      ,    
    output     TFT_LED     ,    

    // ==================== Group B: LED 指示灯 ====================
    input      A0_LED2     ,    
    input      A1_LED3     ,    
    input      STM_UART_RX ,    // 接收 STM32 PA2 串口 (原 A2_LED4 → T22)
    input      B5_LED1     ,    

    output     BASE_LED0   ,    
    output     BASE_LED1   ,    
    output     BASE_LED2   ,    
    output     BASE_LED3   ,    

    // ==================== Group C: 独立按键 ====================
    input      BASE_BTN1   ,    
    input      BASE_BTN2   ,    
    input      BASE_BTN3   ,    
    input      BASE_BTN4   ,    

    input      D7_LCD_RST  ,    // 屏幕复位来自 STM32 PB0 (原 D7_KEY1 → Y20, 改为 INPUT!)
    output     D0_KEY2     ,    
    output     B0_OUT      ,    

    // ==================== Group E: 触摸屏 SPI + IRQ ====================
    input      D4_TCS     ,    
    input      D5_TDIN    ,    
    input      D3_TCLK    ,    

    output     B2_PEN     ,    
    output     B1_DOUT    ,    

    output     TOUCH_CS   ,    
    output     TOUCH_DIN  ,    
    output     TOUCH_CLK  ,    
    input      TOUCH_IRQ  ,    
    input      TOUCH_DO        
);

// ==================== Group A: LCD 信号透传 ====================
assign TFT_RST  = D7_LCD_RST ;  // 修改：TFT复位由 PB0 提供
assign TFT_CS   = A4_LCD_CS  ;
assign TFT_SCK  = A5_LCD_SCK ;
assign TFT_SDI  = B4_LCD_SDA ;
assign TFT_DC   = B3_LCD_DC  ;
assign TFT_LED  = D6_LCD_BL  ;

// ==================== Group B: LED 信号透传 ====================
assign BASE_LED0 = A0_LED2 ;
assign BASE_LED1 = A1_LED3 ;
assign BASE_LED2 = 1'b0;        // PA2 现用作串口，底板 LED2 置空
assign BASE_LED3 = B5_LED1 ;

// ==================== Group C: 按键信号透传 ====================
// PB0 现用作 LCD_RST，因此 KEY1 移除
assign D0_KEY2 = BASE_BTN2 ;
assign B0_OUT  = BASE_BTN3 ; 

// ==================== Group E: 触摸屏信号透传 ====================
assign TOUCH_CS  = D4_TCS  ;
assign TOUCH_DIN = D5_TDIN ;
assign TOUCH_CLK = D3_TCLK ;
assign B2_PEN    = TOUCH_IRQ ;
assign B1_DOUT   = BASE_BTN4 ; 

// =======================================================================
// Zynq PS EMIO UART 模块实例化
// 此模块通过 vivado_setup.tcl 生成的 Block Design (system.bd) 引出
// =======================================================================
system_wrapper zynq_ps_inst (
    .UART_1_0_rxd (STM_UART_RX),
    .UART_1_0_txd (STM_UART_TX)
);

endmodule

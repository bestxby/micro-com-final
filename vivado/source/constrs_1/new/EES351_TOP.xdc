# ============================================================================
# EES351_TOP.xdc — EES-351-MC FPGA 顶层桥接模块约束文件
# 依据: u5 工程源码 (lcd.h/led.h/key.h/my_i2c.h)
#       + 用户 Arduino↔STM32 映射
#       + EES-351-MC 硬件手册 v1.0
# 芯片: xc7z020clg484-1 (Bank 33, LVCMOS33)
# ============================================================================


# ============================================================================
# Group A: TFT LCD SPI 接口 (6 路 → J5 连接器)
# ============================================================================

# --- A-1. Arduino 侧输入 (← STM32 驱动, FPGA 为 INPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports A3_LCD_RST]
set_property IOSTANDARD LVCMOS33 [get_ports A4_LCD_CS]
set_property IOSTANDARD LVCMOS33 [get_ports A5_LCD_SCK]
set_property IOSTANDARD LVCMOS33 [get_ports B4_LCD_SDA]
set_property IOSTANDARD LVCMOS33 [get_ports D6_LCD_BL]
set_property IOSTANDARD LVCMOS33 [get_ports B3_LCD_DC]

# STM32 PA3 → Arduino A3 → U22
set_property PACKAGE_PIN U22  [get_ports A3_LCD_RST]
# STM32 PA4 → Arduino A4 → V22
set_property PACKAGE_PIN V22  [get_ports A4_LCD_CS]
# STM32 PA5 → Arduino A5 → W22
set_property PACKAGE_PIN W22  [get_ports A5_LCD_SCK]
# STM32 PA7 → Arduino B4 → V19
set_property PACKAGE_PIN V19  [get_ports B4_LCD_SDA]
# STM32 PB1 → Arduino D6 → Y19
set_property PACKAGE_PIN Y19  [get_ports D6_LCD_BL]
# STM32 PA8 → Arduino B3 → AA22
set_property PACKAGE_PIN AA22 [get_ports B3_LCD_DC]

# --- A-2. TFT 侧输出 (FPGA 驱动 → J5, OUTPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports TFT_RST]
set_property IOSTANDARD LVCMOS33 [get_ports TFT_CS]
set_property IOSTANDARD LVCMOS33 [get_ports TFT_SCK]
set_property IOSTANDARD LVCMOS33 [get_ports TFT_SDI]
set_property IOSTANDARD LVCMOS33 [get_ports TFT_DC]
set_property IOSTANDARD LVCMOS33 [get_ports TFT_LED]

set_property PACKAGE_PIN N20 [get_ports TFT_RST]   ;# J5-3  TFT 硬件复位
set_property PACKAGE_PIN N19 [get_ports TFT_CS]    ;# J5-1  TFT 片选
set_property PACKAGE_PIN N22 [get_ports TFT_SCK]   ;# J5-7  TFT SPI 时钟
set_property PACKAGE_PIN M22 [get_ports TFT_SDI]   ;# J5-6  TFT SPI MOSI
set_property PACKAGE_PIN M21 [get_ports TFT_DC]    ;# J5-4  TFT 数据/命令
set_property PACKAGE_PIN P22 [get_ports TFT_LED]   ;# J5-9  TFT 背光


# ============================================================================
# Group B: LED 指示灯 (4 路 Arduino → 底板 LED)
# ============================================================================

# --- B-1. Arduino 侧输入 (← STM32 驱动, FPGA 为 INPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports A0_LED2]
set_property IOSTANDARD LVCMOS33 [get_ports A1_LED3]
set_property IOSTANDARD LVCMOS33 [get_ports A2_LED4]
set_property IOSTANDARD LVCMOS33 [get_ports B5_LED1]

set_property PACKAGE_PIN T21  [get_ports A0_LED2]   ;# STM32 PA0  → Arduino A0
set_property PACKAGE_PIN U21  [get_ports A1_LED3]   ;# STM32 PA1  → Arduino A1
set_property PACKAGE_PIN T22  [get_ports A2_LED4]   ;# STM32 PA2  → Arduino A2
set_property PACKAGE_PIN V18  [get_ports B5_LED1]   ;# STM32 PC13 → Arduino B5

# --- B-2. 底板 LED 输出 (FPGA 驱动, OUTPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports BASE_LED0]
set_property IOSTANDARD LVCMOS33 [get_ports BASE_LED1]
set_property IOSTANDARD LVCMOS33 [get_ports BASE_LED2]
set_property IOSTANDARD LVCMOS33 [get_ports BASE_LED3]

set_property PACKAGE_PIN U16 [get_ports BASE_LED0]  ;# 底板 LED0
set_property PACKAGE_PIN U17 [get_ports BASE_LED1]  ;# 底板 LED1
set_property PACKAGE_PIN U15 [get_ports BASE_LED2]  ;# 底板 LED2
set_property PACKAGE_PIN V15 [get_ports BASE_LED3]  ;# 底板 LED3


# ============================================================================
# Group C: 独立按键 (2 路 底板 BTN → Arduino)
# ============================================================================

# --- C-1. 底板按键输入 (← 按键驱动, FPGA 为 INPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports BASE_BTN1]
set_property IOSTANDARD LVCMOS33 [get_ports BASE_BTN2]
set_property IOSTANDARD LVCMOS33 [get_ports BASE_BTN3]
set_property IOSTANDARD LVCMOS33 [get_ports BASE_BTN4]

set_property PACKAGE_PIN H15 [get_ports BASE_BTN1]  ;# 底板按键 PB17 (S0) - Middle
set_property PACKAGE_PIN K15 [get_ports BASE_BTN2]  ;# 底板按键 PB18 (S1) - Up
set_property PACKAGE_PIN J15 [get_ports BASE_BTN3]  ;# 底板按键 PB19 (S2) - Left
set_property PACKAGE_PIN W18 [get_ports BASE_BTN4]  ;# 底板按键 PB20 (S3) - Right

# --- C-2. Arduino 侧输出 (FPGA 驱动 → STM32, OUTPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports D7_KEY1]
set_property IOSTANDARD LVCMOS33 [get_ports D0_KEY2]
set_property IOSTANDARD LVCMOS33 [get_ports B0_OUT]

set_property PACKAGE_PIN Y20  [get_ports D7_KEY1]   ;# → Arduino D7 → STM32 PB0
set_property PACKAGE_PIN Y18  [get_ports D0_KEY2]   ;# → Arduino D0 → STM32 PB8
set_property PACKAGE_PIN AB21 [get_ports B0_OUT]    ;# → Arduino B0 → STM32 PA15


# ============================================================================
# Group E: 触摸屏 SPI + IRQ (Arduino ↔ TFT 触摸面板 J5, 全 5 路)
# ============================================================================

# --- E-1. Arduino 侧: STM32 → FPGA (INPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports D4_TCS]
set_property IOSTANDARD LVCMOS33 [get_ports D5_TDIN]
set_property IOSTANDARD LVCMOS33 [get_ports D3_TCLK]

set_property PACKAGE_PIN AA19 [get_ports D4_TCS]    ;# STM32 PB4 → Arduino D4
set_property PACKAGE_PIN W17  [get_ports D5_TDIN]   ;# STM32 PB3 → Arduino D5
set_property PACKAGE_PIN Y21  [get_ports D3_TCLK]   ;# STM32 PB5 → Arduino D3

# --- E-2. Arduino 侧: FPGA → STM32 (OUTPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports B2_PEN]
set_property IOSTANDARD LVCMOS33 [get_ports B1_DOUT]

set_property PACKAGE_PIN AB22 [get_ports B2_PEN]    ;# → Arduino B2 → STM32 PA11 (IRQ)
set_property PACKAGE_PIN AA21 [get_ports B1_DOUT]   ;# → Arduino B1 → STM32 PA12 (MISO)

# --- E-3. TFT 触摸面板侧: FPGA → J5 (OUTPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports TOUCH_CS]
set_property IOSTANDARD LVCMOS33 [get_ports TOUCH_DIN]
set_property IOSTANDARD LVCMOS33 [get_ports TOUCH_CLK]

set_property PACKAGE_PIN R21 [get_ports TOUCH_CS]   ;# → J5-15 T_CS
set_property PACKAGE_PIN P20 [get_ports TOUCH_DIN]  ;# → J5-16 T_DIN
set_property PACKAGE_PIN M19 [get_ports TOUCH_CLK]  ;# → J5-12 T_CLK

# --- E-4. TFT 触摸面板侧: J5 → FPGA (INPUT) ---
set_property IOSTANDARD LVCMOS33 [get_ports TOUCH_IRQ]
set_property IOSTANDARD LVCMOS33 [get_ports TOUCH_DO]

set_property PACKAGE_PIN P15 [get_ports TOUCH_IRQ]  ;# ← J5-16 T_IRQ
set_property PACKAGE_PIN P21 [get_ports TOUCH_DO]   ;# ← J5-16 T_DO

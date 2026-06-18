# ========== Arduino 侧 (_ARM 输入) IOSTANDARD ==========
set_property IOSTANDARD LVCMOS33 [get_ports CS_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports DC_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports LED_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports RST_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports SCK_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports SDI_ARM]

# ========== TFT 侧 (_BASE 输出) IOSTANDARD ==========
set_property IOSTANDARD LVCMOS33 [get_ports CS_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports DC_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports LED_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports RST_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports SCK_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports SDI_BASE]

# ========== Arduino 侧 (_ARM 输入) PACKAGE_PIN ==========
# 背光 LED   : STM32 PB7 → Arduino D1 → AB20
set_property PACKAGE_PIN AB20 [get_ports LED_ARM]
# 数据/命令 DC: STM32 PA7 → Arduino B4 → V19
set_property PACKAGE_PIN V19  [get_ports DC_ARM]
# 片选 CS     : STM32 PB5 → Arduino D3 → Y21
set_property PACKAGE_PIN Y21  [get_ports CS_ARM]
# 复位 RST    : STM32 PB8 → Arduino D0 → Y18
set_property PACKAGE_PIN Y18  [get_ports RST_ARM]
# SPI 时钟 SCK: STM32 PA5 → Arduino A5 → W22
set_property PACKAGE_PIN W22  [get_ports SCK_ARM]
# SPI 数据 SDI: STM32 PB6 → Arduino D2 → AB19
set_property PACKAGE_PIN AB19 [get_ports SDI_ARM]

# ========== TFT 侧 (_BASE 输出) PACKAGE_PIN ==========
set_property PACKAGE_PIN N19 [get_ports CS_BASE]
set_property PACKAGE_PIN N20 [get_ports RST_BASE]
set_property PACKAGE_PIN M21 [get_ports DC_BASE]
set_property PACKAGE_PIN M22 [get_ports SDI_BASE]
set_property PACKAGE_PIN N22 [get_ports SCK_BASE]
set_property PACKAGE_PIN P22 [get_ports LED_BASE]

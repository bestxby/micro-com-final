# Version1 — LCD 连线工程：STM32 → Arduino → FPGA → TFT

## 工程概述

本工程实现了一个 **直连（透传/连线）** 功能，将 STM32 的 SPI LCD 控制信号通过 Arduino 接口送入 FPGA（xc7z020clg484-1），FPGA 内部不做任何逻辑处理，直接透传至 TFT 液晶屏接口。

**数据流向：**
```
STM32F103C8  →  Arduino 排针  →  FPGA (LCD.v 透传)  →  4英寸 TFT 屏幕
```

---

## 1. STM32 ↔ Arduino 引脚连接

Arduino 接口与 STM32F103C8 的直接连接关系（用户提供）：

| Arduino 引脚 | STM32 引脚 |
|:------------:|:----------:|
| A0 | PA0 |
| A1 | PA1 |
| A2 | PA2 |
| A3 | PA3 |
| A4 | PA4 |
| A5 | PA5 |
| B5 | PC13 |
| B4 | PA7 |
| B3 | PA8 |
| B2 | PA11 |
| B1 | PA12 |
| B0 | PA15 |
| D7 | PB0 |
| D6 | PB1 |
| D5 | PB3 |
| D4 | PB4 |
| D3 | PB5 |
| D2 | PB6 |
| D1 | PB7 |
| D0 | PB8 |

> **注意：** Arduino 接口共有 28 个引脚（见下方完整列表），以上仅列出已确认连接的 20 个引脚。RESET、SCL、SDA、AREF、MISO、SCK、SS、MOSI 等 8 个引脚的 STM32 连接暂未给出。

---

## 2. Arduino → FPGA 引脚约束（完整 28 引脚）

依据 EES-351-MC 微机原理实验箱硬件手册，Arduino 接口全部 FPGA 引脚约束如下（I/O 标准均为 **LVCMOS33**）：

| Arduino 引脚 | J 连接器 | FPGA Pin | 方向 |
|:------------:|:--------:|:--------:|:----:|
| Arduino_A0 | J18-1 | T21 | INOUT |
| Arduino_A1 | J18-2 | U21 | INOUT |
| Arduino_A2 | J18-3 | T22 | INOUT |
| Arduino_A3 | J18-4 | U22 | INOUT |
| Arduino_A4 | J18-5 | V22 | INOUT |
| Arduino_A5 | J18-6 | W22 | INOUT |
| Arduino_B0 | J17-1 | AB21 | INOUT |
| Arduino_B1 | J17-2 | AA21 | INOUT |
| Arduino_B2 | J17-3 | AB22 | INOUT |
| Arduino_B3 | J17-4 | AA22 | INOUT |
| Arduino_B4 | J17-5 | V19 | INOUT |
| Arduino_B5 | J17-6 | V18 | INOUT |
| Arduino_D0 | J19-1 | Y18 | INOUT |
| Arduino_D1 | J19-2 | AB20 | INOUT |
| Arduino_D2 | J19-3 | AB19 | INOUT |
| Arduino_D3 | J19-4 | Y21 | INOUT |
| Arduino_D4 | J19-5 | AA19 | INOUT |
| Arduino_D5 | J19-6 | W17 | INOUT |
| Arduino_D6 | J19-7 | Y19 | INOUT |
| Arduino_D7 | J19-8 | Y20 | INOUT |
| Arduino_RESET | J16-3 | U19 | INOUT |
| Arduino_SCL | J17-10 | W21 | INOUT |
| Arduino_SDA | J17-9 | U20 | INOUT |
| Arduino_AREF | J17-8 | V20 | INOUT |
| Arduino_MISO | J20-1 | AB16 | INOUT |
| Arduino_SCK | J20-3 | AB15 | INOUT |
| Arduino_SS | J20-5 | AA14 | INOUT |
| Arduino_MOSI | J20-4 | AB17 | INOUT |

---

## 3. FPGA → TFT 接口引脚约束

依据 EES-351-MC 硬件手册，TFT 4 英寸 SPI 屏幕接口全部 FPGA 引脚约束如下（I/O 标准均为 **LVCMOS33**）：

| 引脚标号 | J5 连接器 | FPGA Pin | 方向 |
|:--------:|:--------:|:--------:|:----:|
| TFT_CS | J5-1 | N19 | OUTPUT |
| TFT_RESET | J5-3 | N20 | OUTPUT |
| TFT_DC | J5-4 | M21 | OUTPUT |
| TFT_SDI | J5-6 | M22 | OUTPUT |
| TFT_SCK | J5-7 | N22 | OUTPUT |
| TFT_LED | J5-9 | P22 | OUTPUT |
| TFT_SDO | J5-10 | R20 | OUTPUT |
| T_CLK | J5-12 | M19 | OUTPUT |
| T_CS | J5-15 | R21 | OUTPUT |
| T_DIN | J5-16 | P20 | OUTPUT |
| T_DO | J5-16 | P21 | OUTPUT |
| T_IRQ | J5-16 | P15 | OUTPUT |

---

## 4. LCD.v 模块 — 信号透传（LCD 工程实际使用的引脚）

[LCD.v](vivado/source/sources_1/new/LCD.v) 模块将 6 路 Arduino 侧的 SPI 信号直连到 TFT 屏幕：

```verilog
module LCD(
    input      LED_ARM,     // 背光控制
    input      DC_ARM,      // 数据/命令选择
    input      CS_ARM,      // 片选
    input      RST_ARM,     // 复位
    input      SCK_ARM,     // SPI 时钟
    input      SDI_ARM,     // SPI 数据输入

    output     LED_BASE,    // → TFT 背光
    output     DC_BASE,     // → TFT 数据/命令
    output     CS_BASE,     // → TFT 片选
    output     RST_BASE,    // → TFT 复位
    output     SCK_BASE,    // → TFT SPI 时钟
    output     SDI_BASE     // → TFT SPI 数据
);

assign LED_BASE = LED_ARM;
assign DC_BASE  = DC_ARM;
assign CS_BASE  = CS_ARM;
assign RST_BASE = RST_ARM;
assign SCK_BASE = SCK_ARM;
assign SDI_BASE = SDI_ARM;

endmodule
```

---

## 5. 完整信号链路一览

**STM32 → Arduino → FPGA → TFT 的 6 路 SPI 信号全程映射：**

| 信号功能 | STM32 引脚 | Arduino 引脚 | FPGA Pin (输入) | FPGA Pin (输出) | TFT 引脚 |
|:--------:|:----------:|:------------:|:---------------:|:---------------:|:--------:|
| 背光 LED | PB6 | D2 | AB19 | P22 | TFT_LED |
| 数据/命令 DC | PB7 | D1 | AB20 | M21 | TFT_DC |
| 复位 RST | PB8 | D0 | Y18 | N20 | TFT_RESET |
| SPI 时钟 SCK | PA5 | A5 | W22 | N22 | TFT_SCK |
| SPI 数据 SDI | PA7 | B4 | V19 | M22 | TFT_SDI |
| 片选 CS | PB5 | D3 | Y21 | N19 | TFT_CS |

---

## 6. LCD.xdc 约束文件内容

[LCD.xdc](vivado/source/constrs_1/new/LCD.xdc)：

```tcl
# Arduino 侧（_ARM 输入）IOSTANDARD
set_property IOSTANDARD LVCMOS33 [get_ports CS_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports DC_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports LED_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports RST_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports SCK_ARM]
set_property IOSTANDARD LVCMOS33 [get_ports SDI_ARM]

# TFT 侧（_BASE 输出）IOSTANDARD
set_property IOSTANDARD LVCMOS33 [get_ports CS_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports DC_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports LED_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports RST_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports SCK_BASE]
set_property IOSTANDARD LVCMOS33 [get_ports SDI_BASE]

# Arduino 侧 PACKAGE_PIN
set_property PACKAGE_PIN AB19 [get_ports LED_ARM]
set_property PACKAGE_PIN AB20 [get_ports DC_ARM]
set_property PACKAGE_PIN Y18  [get_ports RST_ARM]
set_property PACKAGE_PIN W22  [get_ports SCK_ARM]
set_property PACKAGE_PIN V19  [get_ports SDI_ARM]
set_property PACKAGE_PIN Y21  [get_ports CS_ARM]

# TFT 侧 PACKAGE_PIN
set_property PACKAGE_PIN N19 [get_ports CS_BASE]
set_property PACKAGE_PIN N20 [get_ports RST_BASE]
set_property PACKAGE_PIN M21 [get_ports DC_BASE]
set_property PACKAGE_PIN M22 [get_ports SDI_BASE]
set_property PACKAGE_PIN N22 [get_ports SCK_BASE]
set_property PACKAGE_PIN P22 [get_ports LED_BASE]
```

> ⚠️ 约束文件中存在两个冗余条目 `DIN_ARM` 和 `DIN_BASE`（仅设置了 IOSTANDARD 但未在模块中定义），综合时会报警告。

---

## 7. FPGA 芯片信息

- **芯片型号：** Xilinx ZYNQ-7000 系列 **xc7z020clg484-1**
- **架构：** ARM Cortex-A9 双核 (PS) + 可编程逻辑 (PL)
- **PL 端时钟：** 100 MHz（引脚 AA18）
- **PL Bank 电压：** Bank0/13/33 = 3.3V, Bank34/35 = 1.8V
- **Arduino 接口所在 Bank：** Bank33（LVCMOS33, 3.3V）
- **TFT 接口所在 Bank：** Bank33（LVCMOS33, 3.3V）

---

*基于 EES-351-MC 微机原理实验箱硬件手册 v1.0 (2024.05) 及用户提供的 STM32 连接关系整理*

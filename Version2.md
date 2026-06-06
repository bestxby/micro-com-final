# 工程文档 Version2 — STM32 传感器监测系统 + FPGA LCD 连线桥接

> **项目名称**: 基于 STM32F103C8T6 的传感器监测系统 (AI 自适应异常检测) + FPGA LCD 信号桥接  
> **目标芯片**: STM32F103C8T6 (Cortex-M3) + Xilinx xc7z020clg484-1 (ZYNQ-7000)  
> **开发环境**: Keil uVision 5 (ARM Compiler 6) + Vivado  
> **实验平台**: EES-351-MC 微机原理实验箱 (EES-351F-V1.0 STM32 子板 + ZYNQ 核心板)  
> **FPGA 模块**: EES351_TOP (替代旧版 LCD)  
> **文档版本**: Version2  
> **更新日期**: 2026-05-30  

---

## 1. 系统总体架构

```
┌─────────────┐     Arduino 排针       ┌─────────────┐     底板外设        ┌──────────────┐
│  STM32F103   │ ────────────────────→  │   ZYNQ FPGA  │ ────────────────→ │  4.0 inch    │
│   (C8T6)     │   SPI + LED + KEY      │  (EES351_TOP)│  TFT / LED / BTN  │  TFT LCD     │
│              │                        │              │                   │ (ST7796S)    │
│ 传感器采集    │                        │ 纯连线透传    │                   │ 240×320      │
│ AI 异常检测   │                        │ 4 组 16 信号  │                   │ RGB565       │
│ UI 交互       │                        │              │                   │              │
└─────────────┘                        └─────────────┘                   └──────────────┘
```

**数据流向**: STM32F103 → Arduino 排针 → FPGA (EES351_TOP 透传) → 底板外设 (TFT / LED / BTN)

---

## 2. 完整信号链路：STM32 → Arduino → FPGA → 底板外设

> **依据**: u5 工程源码 (lcd.h / led.h / key.h / my_i2c.h) + 用户提供的 Arduino↔STM32 硬件映射 + EES-351-MC 硬件手册

### 2.1 Group A — TFT LCD SPI (6 路)

| 信号功能 | STM32 | Arduino | FPGA In | → | FPGA Out | TFT |
|:--------:|:-----:|:-------:|:-------:|:-:|:--------:|:---:|
| 复位 RST | **PA3** | A3 | U22 | → | N20 | TFT_RESET |
| 片选 CS | **PA4** | A4 | V22 | → | N19 | TFT_CS |
| 时钟 SCK | **PA5** | A5 | W22 | → | N22 | TFT_SCK |
| 数据/命令 DC | **PA8** | B3 | AA22 | → | M21 | TFT_DC |
| 数据 SDI | **PA7** | B4 | V19 | → | M22 | TFT_SDI |
| 背光 BL | **PB1** | D6 | Y19 | → | P22 | TFT_LED |

> LCD 6 路信号全部走 Arduino，无 PMOD 依赖。DC 信号原为 PA6(PMOD)，已改为 [PA8→B3](u5/Hardware/lcd.h:14)。

### 2.2 Group B — LED 指示灯 (4 路)

| LED | STM32 | Arduino | FPGA In | → | FPGA Out | 底板 LED |
|:---:|:-----:|:-------:|:-------:|:-:|:--------:|:--------:|
| LED2 呼吸灯 | **PA0** | A0 | T21 | → | U16 | BASE_LED0 |
| LED3 预留 | **PA1** | A1 | U21 | → | U17 | BASE_LED1 |
| LED4 预留 | **PA2** | A2 | T22 | → | U15 | BASE_LED2 |
| LED1 告警 | **PC13** | B5 | V18 | → | V15 | BASE_LED3 |

### 2.3 Group C — 独立按键 (4 路五向按键 + 1 路面包板按键 + 触摸 IRQ)

| 按键 / 信号 | 底板 BTN / 器件 | FPGA In | → | FPGA Out | Arduino | STM32 | 功能角色 |
|:----:|:--------:|:-------:|:-:|:--------:|:-------:|:-----:|:---|
| KEY1 | H15 (S0) | H15 | → | Y20 | D7 | **PB0** | 中间键：切换主题 / 硬件自愈重置 |
| KEY2 | K15 (S1) | K15 | → | Y18 | D0 | **PB8** | 向上键：小游戏跳跃与开始 |
| KEY_LEFT | J15 (S2) | J15 | → | AB21 | B0 | **PA15** | 向左键：向左循环翻页 |
| KEY_RIGHT | W18 (S3) | W18 | → | AA21 | B1 | **PA12** | 向右键：向右循环翻页 (复用 B1_DOUT) |
| TOUCH_IRQ | P15 (IRQ) | P15 | → | AB22 | B2 | **PA11** | 触摸中断 IRQ (复用 B2_PEN，供 SD 卡共用) |
| KEY3 | (外接面包板)| - | → | - | - | **PB9** | 面包板外接按键：一键布防/撤防警戒开关 |


### 2.4 Group D — I²C 总线 (直连底板, 不经 FPGA)

| 信号 | STM32 | Arduino | FPGA Pin | 备注 |
|:----:|:-----:|:-------:|:--------:|:----|
| I2C_SCL | **PA6** | (N/A) | (N/A) | AHT20/BMP280/BH1750 直连底板 I²C 总线 (非 Arduino 排针) |
| I2C_SDA | **PB10** | (N/A) | (N/A) | AHT20/BMP280/BH1750 直连底板 I²C 总线 (非 Arduino 排针) |

> I²C 传感器 (AHT20/0x38, BMP280/0x76, BH1750/0x23) 通过底板走线直连，FPGA 不参与路由。

### 2.5 Arduino ↔ STM32 完整映射 (20 引脚)

| Arduino | STM32 | 信号角色 | Arduino | STM32 | 信号角色 |
|:-------:|:-----:|:--------|:-------:|:-----:|:--------|
| A0 | PA0 | LED2 (红) | D7 | PB0 | KEY1 (中间 S0) |
| A1 | PA1 | LED3 (黄) | D6 | PB1 | LCD_BL |
| A2 | PA2 | LED4 (绿) | D5 | PB3 | SD_MOSI (未使用) |
| A3 | PA3 | LCD_RST | D4 | PB4 | SD_CS (未使用) |
| A4 | PA4 | LCD_CS | D3 | PB5 | SD_SCK (未使用) |
| A5 | PA5 | LCD_SCK | D2 | PB6 | WiFi_TXD (重映射) |
| B5 | PC13 | LED1 (WiFi) | D1 | PB7 | WiFi_RXD (重映射) |
| B4 | PA7 | LCD_SDA | D0 | PB8 | KEY2 (向上 S1) |
| B3 | PA8 | LCD_DC | | | |
| B2 | PA11 | TOUCH_IRQ (复用 B2_PEN)| | | |
| B1 | PA12 | KEY_RIGHT (向右 S3)| | | |
| B0 | PA15 | KEY_LEFT (向左 S2) | | | |


> **未确认**: Arduino RESET/SCL/SDA/AREF/MISO/SCK/SS/MOSI 的 STM32 连接暂未给出。

---

## 3. FPGA 侧 — EES351_TOP 桥接模块

> 替代旧版 LCD.v，新增 LED 和按键路由。全部信号走 Arduino 接口。

### 3.1 Verilog 源码

[EES351_TOP.v](vivado/source/sources_1/new/EES351_TOP.v)：

```verilog
`timescale 1ns / 1ps

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
```


### 3.2 FPGA 引脚约束

[EES351_TOP.xdc](vivado/source/constrs_1/new/EES351_TOP.xdc)：

```tcl
# ============================================================================
# EES351_TOP.xdc — EES-351-MC FPGA 顶层桥接模块约束文件
# 芯片: xc7z020clg484-1 (Bank 33, LVCMOS33)
# ============================================================================

# ===== Group A: LCD Arduino 侧 (INPUT) =====
set_property PACKAGE_PIN U22  [get_ports A3_LCD_RST]   ;# PA3 → A3
set_property PACKAGE_PIN V22  [get_ports A4_LCD_CS]    ;# PA4 → A4
set_property PACKAGE_PIN W22  [get_ports A5_LCD_SCK]   ;# PA5 → A5
set_property PACKAGE_PIN AA22 [get_ports B3_LCD_DC]    ;# PA8 → B3
set_property PACKAGE_PIN V19  [get_ports B4_LCD_SDA]   ;# PA7 → B4
set_property PACKAGE_PIN Y19  [get_ports D6_LCD_BL]    ;# PB1 → D6

# ===== Group A: TFT 侧 (OUTPUT) =====
set_property PACKAGE_PIN N20 [get_ports TFT_RST]
set_property PACKAGE_PIN N19 [get_ports TFT_CS]
set_property PACKAGE_PIN N22 [get_ports TFT_SCK]
set_property PACKAGE_PIN M21 [get_ports TFT_DC]
set_property PACKAGE_PIN M22 [get_ports TFT_SDI]
set_property PACKAGE_PIN P22 [get_ports TFT_LED]

# ===== Group B: LED Arduino 侧 (INPUT) =====
set_property PACKAGE_PIN T21  [get_ports A0_LED2]      ;# PA0  → A0
set_property PACKAGE_PIN U21  [get_ports A1_LED3]      ;# PA1  → A1
set_property PACKAGE_PIN T22  [get_ports A2_LED4]      ;# PA2  → A2
set_property PACKAGE_PIN V18  [get_ports B5_LED1]      ;# PC13 → B5

# ===== Group B: 底板 LED (OUTPUT) =====
set_property PACKAGE_PIN U16 [get_ports BASE_LED0]
set_property PACKAGE_PIN U17 [get_ports BASE_LED1]
set_property PACKAGE_PIN U15 [get_ports BASE_LED2]
set_property PACKAGE_PIN V15 [get_ports BASE_LED3]

# ===== Group C: 底板按键 (INPUT) =====
set_property PACKAGE_PIN H15 [get_ports BASE_BTN1]     ;# 底板按键 PB17 (S0) - Middle
set_property PACKAGE_PIN K15 [get_ports BASE_BTN2]     ;# 底板按键 PB18 (S1) - Up
set_property PACKAGE_PIN J15 [get_ports BASE_BTN3]     ;# 底板按键 PB19 (S2) - Left
set_property PACKAGE_PIN W18 [get_ports BASE_BTN4]     ;# 底板按键 PB20 (S3) - Right

# ===== Group C: Arduino 侧按键 (OUTPUT) =====
set_property PACKAGE_PIN Y20  [get_ports D7_KEY1]      ;# → Arduino D7 → STM32 PB0
set_property PACKAGE_PIN Y18  [get_ports D0_KEY2]      ;# → Arduino D0 → STM32 PB8
set_property PACKAGE_PIN AB21 [get_ports B0_OUT]       ;# → Arduino B0 → STM32 PA15

# ===== Group E: 触摸屏 SPI + IRQ =====
set_property PACKAGE_PIN AA19 [get_ports D4_TCS]     ;# STM32 PB4 → Arduino D4
set_property PACKAGE_PIN W17  [get_ports D5_TDIN]    ;# STM32 PB3 → Arduino D5
set_property PACKAGE_PIN Y21  [get_ports D3_TCLK]    ;# STM32 PB5 → Arduino D3
set_property PACKAGE_PIN AB22 [get_ports B2_PEN]     ;# → Arduino B2 → STM32 PA11 (TOUCH_IRQ)
set_property PACKAGE_PIN AA21 [get_ports B1_DOUT]    ;# → Arduino B1 → STM32 PA12 (S3 Button)

set_property PACKAGE_PIN R21 [get_ports TOUCH_CS]
set_property PACKAGE_PIN P20 [get_ports TOUCH_DIN]
set_property PACKAGE_PIN M19 [get_ports TOUCH_CLK]
set_property PACKAGE_PIN P15 [get_ports TOUCH_IRQ]
set_property PACKAGE_PIN P21 [get_ports TOUCH_DO]

# 全部 IOSTANDARD: LVCMOS33
```


### 3.3 FPGA 芯片信息

| 参数 | 规格 |
|:---|:---|
| 芯片型号 | Xilinx ZYNQ-7000 **xc7z020clg484-1** |
| 架构 | ARM Cortex-A9 双核 (PS) + 可编程逻辑 (PL) |
| PL 端时钟 | 100 MHz (引脚 AA18) |
| PL Bank 电压 | Bank0/13/33 = 3.3V, Bank34/35 = 1.8V |
| 本模块 I/O 所在 Bank | Bank33 (LVCMOS33, 3.3V) |

---

## 4. STM32 侧 — u5 传感器监测工程

### 4.1 工程目录结构

```
u5/
├── User/                           # 应用层：主程序、AI 算法、异常日志、字体数据
│   ├── main.c                      # 主程序入口 (系统初始化、主循环、UI 刷新、传感器轮询)
│   ├── main.h                      # 主程序头文件 (外部公共接口声明)
│   ├── ai_detect.c                 # AI 自适应温度异常检测算法实现 (EMA 滤波 + 基准学习)
│   ├── ai_detect.h                 # AI 检测器数据结构与状态定义
│   ├── anomaly_log.c               # 异常事件环形日志缓冲区实现
│   ├── anomaly_log.h               # 日志数据结构与接口声明
│   ├── font.h                      # ASCII 8×16 点阵字模数据 (用于 LCD 字符显示)
│   └── stm32f10x_conf.h           # User 层标准外设库头文件聚合
│
├── Hardware/                       # 硬件驱动层：外设与传感器底层驱动
│   ├── led.c / led.h               # LED 驱动 (表驱动架构，支持 4 路 LED)
│   ├── key.c / key.h               # 独立按键驱动 (软件消抖, PB0/PB8)
│   ├── lcd.c / lcd.h               # 4.0 inch SPI LCD 驱动 (ST7796S/ILI9341 兼容)
│   ├── my_i2c.c / my_i2c.h         # 软件模拟 I²C 总线驱动 (PB6/PB7)
│   ├── aht20.c / aht20.h           # AHT20 温湿度传感器驱动 (I²C, 地址 0x38)
│   ├── bmp280.c / bmp280.h         # BMP280 气压传感器驱动 (I²C, 地址 0x76)
│   └── stm32f10x_conf.h           # Hardware 层标准外设库头文件聚合
│
├── Library/                        # 库文件层：CMSIS 核心头文件 + 标准外设库头文件
│   ├── core_cm3.h                  # ARM Cortex-M3 内核外设访问层头文件
│   ├── stm32f10x.h                 # STM32F10x 系列顶层头文件
│   ├── system_stm32f10x.h          # 系统时钟配置函数声明
│   └── (GPIO/RCC/TIM/I2C/SPI/USART/DMA/EXTI/... 标准外设库头文件)
│
├── RTE/                            # Keil RTE 运行时环境
│   └── Device/STM32F103C8/
│       ├── startup_stm32f10x_md.s  # 启动文件 (堆栈初始化、中断向量表)
│       └── system_stm32f10x.c      # 系统时钟配置 (8MHz HSE → 72MHz PLL)
│
├── Box_datasheet/                  # 硬件数据手册与参考资料
│   ├── EES351-MC微机原理实验箱硬件手册20241128.doc  # 实验箱硬件手册
│   ├── ees-351f-1v0.pdf            # STM32 子板原理图 (EES-351F-V1.0)
│   ├── EES-351-MC-V11_0807.pdf     # 底板原理图 (EES-351-MC-V1.1)
│   ├── ZYNQ核心板EES-351-V0.2-0726_forout.pdf      # Zynq 核心板原理图
│   ├── 4.0inch_SPI_Module_MSP4020&MSP4021_User_Manual_CN.pdf  # LCD 模组用户手册
│   ├── ST7796S_datasheet.pdf       # LCD 驱动 IC 数据手册
│   └── AHT20技术手册.pdf           # AHT20 传感器数据手册
│
├── output/                         # 编译输出目录 (AC6 编译产物)
│   ├── demo.axf / demo.hex / demo.map  # 可执行文件 / HEX 固件 / 内存映射
│   └── ...
│
├── demo.uvprojx                    # Keil uVision 5 工程主配置文件
├── demo.uvoptx                     # Keil 工程选项配置
├── README.md                       # 项目说明文档
├── design_report.md                # 课程设计报告
└── week10_lab_assignment.md        # 第 10 周实验任务说明
```

### 4.2 STM32F103C8T6 引脚资源分配（u5 工程原版）

#### 4.2.1 LED 指示灯模块

| LED 编号 | STM32 引脚 | Arduino 连接 | 有效电平 | GPIO 模式 | 功能描述 |
|:---:|:---:|:---|:---:|:---|:---|
| **LED1** | **PC13** | B5 → 底板 LEDx | 低电平 (0) | 推挽输出, 50MHz | 异常告警指示灯：正常时常灭；检测到异常时 ~5Hz 快速闪烁 |
| **LED2** | **PA0** | A0 → 底板 LEDx | 高电平 (1) | 复用推挽输出, 50MHz | TIM2_CH1 PWM 呼吸灯（~720Hz PWM）；异常时强制熄灭 |
| **LED3** | **PA1** | A1 → 底板 LEDx | 高电平 (1) | 推挽输出, 50MHz | 预留外接 LED |
| **LED4** | **PA2** | A2 → 底板 LEDx | 高电平 (1) | 推挽输出, 50MHz | 预留外接 LED |

#### 4.2.2 独立按键输入模块

| 按键编号 | STM32 引脚 | Arduino 连接 | 触发方式 | GPIO 模式 | 功能描述 |
|:---:|:---:|:---|:---:|:---|:---|
| **KEY1** | **PB0** | D7 → 底板 BTN1 | 低电平按下 | 上拉输入 | UI 页面切换 (Page 0→1→2) |
| **KEY2** | **PB8** | D0 → 底板 BTN2 | 低电平按下 | 上拉输入 | 系统复位：重置 AI 学习参数、强制重新初始化传感器 |

#### 4.2.3 I²C 总线模块（软件模拟）

| 信号名称 | STM32 引脚 | Arduino 连接 | GPIO 模式 | 功能描述 |
|:---|:---:|:---|:---|:---|
| **I2C_SCL** | **PA6** | (N/A) | 开漏输出, 50MHz | I²C 时钟线 → AHT20 SCL / BMP280 SCK / BH1750 SCL (非 Arduino) |
| **I2C_SDA** | **PB10** | (N/A) | 开漏输出, 50MHz | I²C 数据线 → AHT20 SDA / BMP280 SDI / BH1750 SDA (非 Arduino) |

> 挂载设备: AHT20 (地址 0x38) + BMP280 (地址 0x76) + BH1750 (地址 0x23)，通过不同 7 位从机地址区分。

#### 4.2.4 传感器子模块

**AHT20 温湿度传感器：**
| 参数 | 规格 |
|:---|:---|
| 挂载总线 | I²C (软件模拟, PA6/PB10) |
| 7 位从机地址 | `0x38` |
| 测量范围 | 温度 -40 ~ +85°C, 湿度 0 ~ 100%RH |
| 典型精度 | 温度 ±0.3°C, 湿度 ±2%RH |

**BMP280 气压传感器：**
| 参数 | 规格 |
|:---|:---|
| 挂载总线 | I²C (软件模拟, PA6/PB10) |
| 7 位从机地址 | `0x76` |
| 测量范围 | 气压 300 ~ 1100 hPa, 温度 -40 ~ +85°C |
| 典型精度 | 气压 ±0.12 hPa, 温度 ±0.5°C |

**BH1750 光强传感器：**
| 参数 | 规格 |
|:---|:---|
| 挂载总线 | I²C (软件模拟, PA6/PB10) |
| 7 位从机地址 | `0x23` |
| 测量范围 | 1 ~ 65535 lx |
| 典型精度 | 1 lx (高分辨率模式) |

#### 4.2.5 LCD 显示屏模块

> **显示器件**: MSP4020 4.0 inch SPI TFT LCD  
> **驱动 IC**: ST7796S (兼容 ILI9341)  
> **分辨率**: 240 × 320 (RGB565)  
> **源码**: [lcd.h](u5/Hardware/lcd.h) / [lcd.c](u5/Hardware/lcd.c)

| 信号名称 | STM32 | Arduino | FPGA In | FPGA Out | TFT | GPIO 模式 |
|:---|:---:|:---:|:---:|:---:|:---|:---|
| LCD_RST | **PA3** | A3 | U22 | N20 | TFT_RESET | 推挽输出 |
| LCD_CS | **PA4** | A4 | V22 | N19 | TFT_CS | 推挽输出 |
| LCD_SCL | **PA5** | A5 | W22 | N22 | TFT_SCK | 推挽输出 |
| LCD_DC | **PA8** | B3 | AA22 | M21 | TFT_DC | 推挽输出 |
| LCD_SDA | **PA7** | B4 | V19 | M22 | TFT_SDI | 推挽输出 |
| LCD_BL | **PB1** | D6 | Y19 | P22 | TFT_LED | 推挽输出 |

> LCD_DC 原为 PA6(PMOD)，已改为 **PA8→Arduino B3**，使全部 6 路信号统一走 Arduino 接口。参见 [lcd.h:14](u5/Hardware/lcd.h#L14)。

### 4.3 GPIO 端口汇总（按端口分组）

#### GPIOA 端口

| 引脚 | 功能模块 | 信号角色 | 模式 | Arduino |
|:---:|:---|:---|:---|:---:|
| **PA0** | LED2 | TIM2_CH1 PWM 呼吸灯 | 复用推挽 | A0 |
| **PA1** | LED3 | GPIO 输出 (预留) | 推挽输出 | A1 |
| **PA2** | LED4 | GPIO 输出 (预留) | 推挽输出 | A2 |
| **PA3** | LCD | LCD_RST (硬件复位) | 推挽输出 | A3 |
| **PA4** | LCD | LCD_CS (SPI 片选) | 推挽输出 | A4 |
| **PA5** | LCD | LCD_SCL (SPI 时钟) | 推挽输出 | A5 |
| **PA6** | I²C 总线 | I2C_SCL (时钟线) | 开漏输出 | (N/A) |
| **PA7** | LCD | LCD_SDA (SPI MOSI) | 推挽输出 | B4 |
| **PA8** | LCD | LCD_DC (数据/命令) | 推挽输出 | B3 |
| **PA11** | SD 卡 | SD_MISO (数据输出) | 上拉输入 | (N/A) |

#### GPIOB 端口

| 引脚 | 功能模块 | 信号角色 | 模式 | Arduino |
|:---:|:---|:---|:---|:---:|
| **PB0** | KEY1 | 按键输入 (页面切换) | 上拉输入 | D7 |
| **PB1** | LCD | LCD_BL (背光控制) | 推挽输出 | D6 |
| **PB3** | SD 卡 | SD_MOSI (数据输入) | 推挽输出 | D3 |
| **PB4** | SD 卡 | SD_CS (片选) | 推挽输出 | D4 |
| **PB5** | SD 卡 | SD_SCK (时钟) | 推挽输出 | D5 |
| **PB6** | (未使用) | 原 I2C_SCL 已释放 | - | - |
| **PB7** | (未使用) | 原 I2C_SDA 已释放 | - | - |
| **PB8** | KEY2 | 按键输入 (系统复位) | 上拉输入 | D0 |
| **PB10** | I²C 总线 | I2C_SDA (数据线) | 开漏输出 | (N/A) |

#### GPIOC 端口

| 引脚 | 功能模块 | 信号角色 | 模式 | Arduino |
|:---:|:---|:---|:---|:---:|
| **PC13** | LED1 | 异常告警闪烁 (低有效) | 推挽输出 | B5 |

### 4.4 引脚占用统计

| GPIO 端口 | 已用引脚数 | 已用引脚列表 | 剩余可用 |
|:---:|:---:|:---|:---:|
| **GPIOA** | 10 / 16 | PA0~PA8, PA11 | PA9, PA10, PA12~PA15 |
| **GPIOB** | 7 / 16 | PB0, PB1, PB3~PB5, PB8, PB10 | PB2, PB6, PB7, PB9, PB11~PB15 |
| **GPIOC** | 1 / 16 | PC13 | PC0~PC12, PC14, PC15 |
| **合计** | **18 / 48** | — | **30 引脚空闲** |

### 4.5 片内外设资源占用

| 外设 | 用途 | 关键配置 |
|:---|:---|:---|
| **TIM2** | LED2 PWM 呼吸灯 (PA0, CH1) | PSC=999, ARR=99, PWM 模式 1, ~720Hz |
| **RCC** | 系统时钟 72MHz (HSE 8MHz → PLL ×9) | system_stm32f10x.c 配置 |

### 4.6 系统软件架构

```
┌─────────────────────────────────────────────────────────┐
│                     User (应用层)                        │
│  main.c  ───  系统主循环 (10ms 心跳, UI 刷新, 按键调度)   │
│  ai_detect.c  ─── AI 异常检测引擎 (EMA + 基准学习)       │
│  anomaly_log.c  ─── 异常事件环形日志 (5 条容量)          │
│  font.h  ───  ASCII 8×16 点阵字模库                     │
├─────────────────────────────────────────────────────────┤
│                   Hardware (驱动层)                      │
│  lcd.c   ─── SPI LCD 图形库 (画点/线/矩/圆/字符/字符串)   │
│  led.c   ─── LED 表驱动控制 + TIM2 PWM 呼吸              │
│  key.c   ─── 按键扫描 + 20ms 软件消抖                    │
│  my_i2c.c  ─── 软件模拟 I²C 主机 (起始/停止/收发/ACK)     │
│  aht20.c   ─── AHT20 校准 + 温湿度读取 (0x38)            │
│  bmp280.c  ─── BMP280 校准补偿 + 气压温度读取 (0x76)      │
├─────────────────────────────────────────────────────────┤
│                   Library (库层)                         │
│  CMSIS Core (core_cm3.h)  +  STM32F10x 标准外设库头文件   │
├─────────────────────────────────────────────────────────┤
│                   RTE (运行时环境)                        │
│  startup_stm32f10x_md.s  +  system_stm32f10x.c          │
└─────────────────────────────────────────────────────────┘
```

### 4.7 系统功能概述

1. **传感器数据采集**: 通过软件 I²C 总线周期性轮询 AHT20 (温湿度)、BMP280 (气压/温度) 和 BH1750 (光照度)，采样间隔约 1.5 秒。
2. **传感器热插拔自愈**: 连续 3 次通信失败自动标记掉线；掉线后每次轮询周期自动尝试重新初始化传感器以恢复连接。
3. **AI 自适应基准学习**: EMA (指数移动平均) 数字滤波 + 渐进式环境温度基准学习（100 个样本约 150 秒），学习完成后自动进入异常监测模式。
4. **实时异常检测**: 滤波后温度偏离学习基准超过 ±5°C 时触发异常告警。
5. **三页 UI 交互**: Page 0 实时监测数据面板, Page 1 AI 学习状态 + 偏差折线图 + 进度条, Page 2 历史异常日志记录 (环形缓冲区, 最多 5 条)。
6. **双 LED 状态指示**: 正常模式 — PA0 呼吸灯渐变；异常模式 — PC13 高频闪烁 (5Hz)。

---

## 5. 硬件平台信息汇总

| 组件 | 型号 | 说明 |
|:---|:---|:---|
| 实验箱 | EES-351-MC | 依元素科技 ZYNQ-7000 基础教学平台 |
| FPGA 核心板 | EES-351-V0.2 | xc7z020clg484-1, DDR3×2 |
| STM32 子板 | EES-351F-V1.0 | STM32F103C8T6 Cortex-M3 |
| LCD 屏幕 | MSP4020 4.0 inch SPI TFT | ST7796S 驱动 IC, 240×320 RGB565 |
| 温湿度传感器 | AHT20 | I²C, 地址 0x38 |
| 气压传感器 | BMP280 | I²C, 地址 0x76 |
| 光强传感器 | BH1750 | I²C, 地址 0x23 |

---

*基于 EES-351-MC 硬件手册 v1.0 (2024.05) + u5 工程源码 (lcd.h/led.h/key.h/my_i2c.h) + 用户 STM32↔Arduino 映射。FPGA 模块: EES351_TOP。旧版 LCD.v/LCD.xdc 保留于 vivado/source/ 供参考。*

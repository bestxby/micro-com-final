# STM32F103C8T6 LED 驱动工程模板 (README.md)

基于 **STM32F103C8T6** (Blue Pill) 的 LED 驱动控制项目，采用模块化设计，划分了 **User** (应用层)、**Hardware** (外设驱动层) 和 **Library** (库文件层)。

本项目全面升级支持 **Keil5** 及 **ARM Compiler 6 (AC6)** 编译器。

---

## 1. 项目目录结构

```
├── User/               # 应用层：存放 main.c 主程序入口及中断处理
├── Hardware/           # 硬件驱动层：LED 驱动、传感器驱动等自定义硬件驱动
│   ├── led.c           # LED 驱动实现（修复了 Toggle 翻转 Bug）
│   └── led.h           # LED 配置与 API 定义
├── Library/            # 库层：预留用于存放 CMSIS、标准外设库或第三方库
├── RTE/                # Keil RTE 运行时环境（自动管理启动文件和系统配置）
└── demo.uvprojx        # Keil uVision5 工程配置文件
```

---

## 2. 软硬件开发环境

### 2.1 软件要求
* **集成开发环境 (IDE)**: Keil uVision 5
* **编译器 (Compiler)**: ARM Compiler 6 (AC6)
* **包含路径 (Include Paths)**: 已配置 `.\User`、`.\Hardware`、`.\Library`
* **字符编码 (Encoding)**: 所有源文件均使用 **UTF-8** 编码

### 2.2 默认硬件接线
| LED 索引 | GPIO 端口 | 引脚号 | 有效电平 | 说明 |
| :--- | :--- | :--- | :--- | :--- |
| **LED1 (Index 0)** | GPIOC | PC13 | 低电平有效 (0) | 系统板载 LED |
| **LED2 (Index 1)** | GPIOA | PA0 | 高电平有效 (1) | 外接 LED |
| **LED3 (Index 2)** | GPIOA | PA1 | 高电平有效 (1) | 外接 LED |
| **LED4 (Index 3)** | GPIOA | PA2 | 高电平有效 (1) | 外接 LED |

---

## 3. 驱动 API 接口

驱动位于 `Hardware/led.h`，提供了标准的表驱动控制 API：
```c
void LED_Init(void);                          // 初始化所有配置的 LED 引脚
void LED_On(uint8_t index);                   // 点亮指定索引的 LED
void LED_Off(uint8_t index);                  // 熄灭指定索引的 LED
void LED_Toggle(uint8_t index);               // 翻转指定索引的 LED 电平（支持高/低电平有效）
void LED_Write(uint8_t index, uint8_t state); // 显式写入 LED 状态（0 = 灭，非 0 = 亮）
```

---

## 4. 快速开始与编译

1. **克隆项目**到本地工作目录。
2. 使用 **Keil uVision 5** 打开根目录下的 [demo.uvprojx](file:///d:/MICROCOMPUTER/micro-com-final/demo.uvprojx) 工程文件。
3. 检查编译选项，确保 **Target -> ARM Compiler** 选择的是 **Version 6**。
4. 点击 **Build (F7)** 进行编译。
5. 通过 DAP-Link / ST-Link / J-Link 将生成的固件烧录至 STM32F103C8T6 核心板，即可观察到 PC13 与 PA0 处的 LED 闪烁效果。

---

## 5. 实验平台：EES-351-MC 微机原理实验箱

本项目实际运行在 **EES-351-MC 微机原理实验箱** 上，该实验箱采用 **STM32F103C8T6 + XC7Z020 Zynq** 双核心架构，通过 80Pin 连接器对接丰富的板上外设。

### 5.1 实验箱整体架构

```
EES-351-MC 底板 (Motherboard)
├── J1/J2 80Pin 连接器 → 对接 Zynq 核心板 (XC7Z020-CLG484)
├── STM32 子板插座       → 对接 STM32 核心板 (EES-351F-V1.0, STM32F103C8T6)
└── 板上外设             → 按键/LED/数码管/TFT/电机/ADC/DAC/PMOD 等
```

### 5.2 STM32 子板 (EES-351F-V1.0) 主要资源

| 资源 | 型号/参数 |
|---|---|
| **主控芯片** | STM32F103C8T6 (Cortex-M3, 64KB Flash, 20KB SRAM) |
| **USB-UART** | CH340K (Type-C 接口, 一键下载) |
| **主晶振** | 8MHz (HSE) |
| **RTC 晶振** | 32.768KHz (LSE) |
| **LDO 稳压** | SPX3819M5-L-3-3 (5V → 3.3V) |
| **板载引出** | Arduino UNO 兼容排针 + PMOD 接口 |

### 5.3 底板 (EES-351-MC-V1.1) 外设资源一览

| 类别 | 板上器件 | 关键参数 |
|---|---|---|
| **按键** | 13 个轻触按键 (BTN1~BTN13) | 其中 BTN1(B8) 为 PB0, BTN2(B9) 为 PB8 |
| **LED** | 8 个独立 LED (LED1~LED8) | 高电平点亮，1K 限流电阻 |
| **拨码开关** | 8 位 DIP 开关 (SW1~SW8) | 上拉接 +1.8V |
| **TFT 彩屏** | 4.0 inch SPI LCD (MSP4020) | 驱动 IC ST7796S, 480×320, RGB565 |
| **HDMI 输出** | HDMI TX (D0±/D1±/D2±/CK±) | LVDS 差分信号 |
| **7 段数码管** | 8 位共阳数码管 (HLS2011AX) | 位选 8 路 + 段选 8 路 |
| **步进电机** | DRV8833PWPR 驱动 | 4 线步进电机接口 (15BY) |
| **直流电机** | DRV8837DSGR 驱动 | 2 线直流电机接口 (R300C) |
| **音频编解码** | ADAU1761BCPZ | 立体声 ADC/DAC, I²S 接口 |
| **蓝牙模块** | BLE-CC41-A (CC2541) | BLE 4.0, UART 透传 |
| **ADC** | ADC0809CCN | 8 通道 8 位, SAR 型 |
| **DAC** | DAC0832 (TYX-DAC0832) | 8 位乘法型 DAC |
| **PMOD 接口** | 4 组 12Pin PMOD (J12~J15) | 3.3V 标准数字扩展口 |
| **Arduino 接口** | Arduino UNO 兼容排针 | D0~D7, A0~A5, SPI, I²C |
| **XADC** | 8 路 SMA 模拟输入 | 带电位器调节 (B103) |
| **电源方案** | TPS563231 ×2 | 5V → 3.3V / 5V → 1.8V, DC 5V 输入 |

### 5.4 本项目实际使用的引脚映射

基于 STM32 子板 Arduino 排针与底板飞线，实际接线如下：

| STM32 引脚 | 物理连接 | 功能角色 | 复用功能 |
|---|---|---|---|
| **PC13** | 板载 LED / 底板 LEDx | LED1 — 异常告警 5Hz 闪烁 | GPIO 推挽输出 |
| **PA0** | 板载 A0 排针 → LEDx | LED2 — PWM 呼吸灯 | TIM2_CH1 |
| **PA1** | 板载 A1 排针 → LEDx | LED3 — 预留 | GPIO 推挽输出 |
| **PA2** | 板载 A2 排针 → LEDx | LED4 — 预留 | GPIO 推挽输出 |
| **PB0** | 板载 D0 排针 → BTN1 | KEY1 — UI 页面切换 | GPIO 上拉输入 |
| **PB8** | 板载 D7 排针 → BTN2 | KEY2 — 重置 AI 学习基线 | GPIO 上拉输入 |
| **PA3** | 板载 A3 排针 → TFT RST | LCD 硬件复位 | GPIO 推挽输出 |
| **PA4** | 板载 A4 排针 → TFT CS | LCD 片选 (SPI CS) | GPIO 推挽输出 |
| **PA5** | 板载 A5 排针 → TFT SCK | LCD SPI 时钟 | GPIO 推挽输出 |
| **PA6** | PMOD 排针 → TFT DC | LCD RS 命令/数据选择 | GPIO 推挽输出 |
| **PA7** | 板载 B1/SDA 排针 → TFT SDA | LCD SPI MOSI 数据线 | GPIO 推挽输出 |
| **PB1** | 板载 D1 排针 → TFT LED | LCD 背光控制 | GPIO 推挽输出 |
| **PB6** | 板载 D5 排针 → AHT20/BMP280 SCL | I²C 时钟线 (软件模拟) | GPIO 开漏输出 |
| **PB7** | 板载 D6 排针 → AHT20/BMP280 SDA | I²C 数据线 (软件模拟) | GPIO 开漏输出 |

### 5.5 外接传感器参数

| 传感器 | 型号 | 通信接口 | I²C 地址 | 测量范围 | 精度 |
|---|---|---|---|---|---|
| **温湿度** | AHT20 | I²C (软件模拟) | 0x38 | 温度 -40~85°C, 湿度 0~100%RH | ±0.3°C, ±2%RH |
| **气压** | BMP280 | I²C (软件模拟) | 0x76 | 300~1100 hPa, -40~85°C | ±0.12hPa, ±0.5°C |

### 5.6 LCD 显示屏参数

| 参数 | 规格 |
|---|---|
| **型号** | MSP4020 4.0 inch SPI TFT |
| **驱动 IC** | ST7796S |
| **分辨率** | 480 × 320 (本项目使用竖屏 240×320) |
| **色深** | 65K 色 (RGB565, 16-bit) |
| **接口** | 4 线 SPI (SCK/SDA/CS/DC) |
| **触控** | XPT2046 电阻触摸 (本项目未启用) |
| **供电** | 3.3V / 5V 兼容 |

---

## 6. 多 Agent 协作规范
如果您是 AI 辅助编码 Agent，请在开发前仔细阅读 [agent.md](file:///d:/MICROCOMPUTER/micro-com-final/agent.md)，了解头文件路径配置、工程文件修改规范以及代码编写标准。

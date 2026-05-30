# STM32F103C8T6 异常检测与娱乐系统工程 (README.md)

基于 **STM32F103C8T6** (Blue Pill) 的环境自适应异常监测与像素飞鸟小游戏系统，采用模块化设计，划分了 **User** (应用层)、**Hardware** (外设驱动层) 和 **Library** (库文件层)。

本项目全面升级支持 **Keil5** 及 **ARM Compiler 6 (AC6)** 编译器，并且适配了 **480×320 横屏**显示。

---

## 1. 项目目录结构

```
├── User/               # 应用层：存放 main.c 主程序入口、AI 突变检测算法、历史日志及飞鸟游戏
│   ├── main.c          # 系统主循环 (10ms 心跳, 两栏式横屏 UI, 按键防丢包缓存)
│   ├── game.c          # 飞鸟小游戏核心实现 (物理模型、碰撞检测、30ms 增量像素刷新)
│   ├── ai_detect.c     # AI 异常检测引擎 (EMA 指数滑动平均 + 渐进式基准学习)
│   └── anomaly_log.c   # 异常事件环形日志缓冲区 (最大容量 5 条)
├── Hardware/           # 硬件驱动层：LED 驱动、传感器驱动、LCD 驱动等
│   ├── lcd.c           # ST7796S 480×320 横屏驱动 (优化了单点及大面积快速填充)
│   ├── led.c           # LED 表驱动控制 + TIM2 CH1 PWM 呼吸灯
│   ├── key.c           # 按键扫描 + 20ms 软件消抖 (单次触发模式)
│   ├── aht20.c         # AHT20 温湿度传感器驱动
│   └── bmp280.c        # BMP280 气压/温度传感器驱动
├── Library/            # 库层：存放 CMSIS 核心文件及标准外设库
├── RTE/                # Keil RTE 运行时环境（自动管理启动文件和系统配置）
└── demo.uvprojx        # Keil uVision5 工程配置文件 (已注册 game.c 并开启 C99 支持)
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
| **LED1 (Index 0)** | GPIOC | PC13 | 低电平有效 (0) | 系统板载 LED / 异常告警 5Hz 闪烁 |
| **LED2 (Index 1)** | GPIOA | PA0 | 高电平有效 (1) | TIM2 PWM 呼吸指示灯 |
| **LED3 (Index 2)** | GPIOA | PA1 | 高电平有效 (1) | AI 自适应学习状态指示灯 (学习中常亮) |
| **LED4 (Index 3)** | GPIOA | PA2 | 高电平有效 (1) | AI 监测状态指示灯 (监测中常亮) |

---

## 3. 驱动 API 接口

LED 驱动位于 `Hardware/led.h`，提供了标准的表驱动控制 API：
```c
void LED_Init(void);                          // 初始化所有配置的 LED 引脚
void LED_On(uint8_t index);                   // 点亮指定索引的 LED (0=LED1, 1=LED2, 2=LED3, 3=LED4)
void LED_Off(uint8_t index);                  // 熄灭指定索引的 LED
void LED_Toggle(uint8_t index);               // 翻转指定索引的 LED 电平
void LED_Write(uint8_t index, uint8_t state); // 写入 LED 状态 (0 = 灭，非 0 = 亮)
```

---

## 4. 快速开始与编译

1. **克隆项目**到本地工作目录。
2. 使用 **Keil uVision 5** 打开根目录下的 [demo.uvprojx](file:///d:/MICROCOMPUTER/micro-com-final/demo.uvprojx) 工程文件。
3. 检查编译选项，确保 **Target -> ARM Compiler** 选择的是 **Version 6**。
4. 点击 **Build (F7)** 进行编译。
5. 通过 DAP-Link / ST-Link / J-Link 将生成的固件烧录至 STM32F103C8T6 核心板。
6. 按下开发板上的 **KEY1 (PB0)** 可进行翻页操作；在小游戏页上按下 **KEY2 (PB8)** 控制飞鸟跳跃。

---

## 5. 实验平台：EES-351-MC 微机原理实验箱

本项目实际运行在 **EES-351-MC 微机原理实验箱** 上，该实验箱采用 **STM32F103C8T6 + XC7Z020 Zynq** 双核心架构。

### 5.1 实验箱整体架构

```
EES-351-MC 底板 (Motherboard)
├── J1/J2 80Pin 连接器 → 对接 Zynq 核心板 (XC7Z020-CLG484)
├── STM32 子板插座       → 对接 STM32 核心板 (EES-351F-V1.0, STM32F103C8T6)
└── 板上外设             → 按键/LED/数码管/TFT/电机/ADC/DAC/PMOD 等
```

### 5.2 本项目实际使用的引脚映射

基于 STM32 子板 Arduino 排针与底板飞线，实际接线如下：

| STM32 引脚 | 物理连接 | 功能角色 | 复用功能 |
|---|---|---|---|
| **PC13** | 板载 LED / 底板 LEDx | LED1 — 异常告警 5Hz 闪烁 | GPIO 推挽输出 |
| **PA0** | 板载 A0 排针 → LEDx | LED2 — PWM 呼吸灯 | TIM2_CH1 |
| **PA1** | 板载 A1 排针 → LEDx | LED3 — AI 学习指示灯 | GPIO 推挽输出 |
| **PA2** | 板载 A2 排针 → LEDx | LED4 — AI 监测指示灯 | GPIO 推挽输出 |
| **PB0** | 板载 D0 排针 → BTN1 | KEY1 — UI 页面切换 (循环翻页) | GPIO 上拉输入 |
| **PB8** | 板载 D7 排针 → BTN2 | KEY2 — 警报重置 / 飞鸟跳跃与重启 | GPIO 上拉输入 |
| **PA3** | 板载 A3 排针 → TFT RST | LCD 硬件复位 | GPIO 推挽输出 |
| **PA4** | 板载 A4 排针 → TFT CS | LCD 片选 (SPI CS) | GPIO 推挽输出 |
| **PA5** | 板载 A5 排针 → TFT SCK | LCD SPI 时钟 | GPIO 推挽输出 |
| **PA8** | 板载 B3 排针 → TFT DC | LCD RS 命令/数据选择 (已修正) | GPIO 推挽输出 |
| **PA7** | 板载 B4 排针 → TFT SDI | LCD SPI MOSI 数据线 | GPIO 推挽输出 |
| **PB1** | 板载 D6 排针 → TFT LED | LCD 背光控制 | GPIO 推挽输出 |
| **PB6** | 板载 D5 排针 → AHT20/BMP280 SCL | I²C 时钟线 (软件模拟) | GPIO 开漏输出 |
| **PB7** | 板载 D6 排针 → AHT20/BMP280 SDA | I²C 数据线 (软件模拟) | GPIO 开漏输出 |

### 5.3 LCD 显示屏参数

| 参数 | 规格 |
|---|---|
| **型号** | MSP4020 4.0 inch SPI TFT |
| **驱动 IC** | ST7796S |
| **分辨率** | 480 × 320 (本项目已完全适配横屏 480×320) |
| **色深** | 65K 色 (RGB565, 16-bit) |
| **接口** | 4 线 SPI (SCK/SDA/CS/DC) |

---

## 6. 系统核心功能设计

### 6.1 传感器热插拔自愈与重连
系统周期（1.5s）读取温湿度与气压数据，当连续通信失败 3 次判定传感器掉线并高亮报错。掉线后系统会自动在每个轮询期重新执行初始化，一旦传感器重新插上可实现瞬时零延迟重连。

### 6.2 AI 自适应基准学习与突变检测
传感器正常工作后，系统使用 EMA 指数移动平均平滑滤波，并在前 100 次采样（约 150 秒）内自动采集拟合环境温度基线。学习完成后，若温度偏离基准超过 ±5°C，系统会自动判定为异常突变并记录日志。

### 6.3 两栏式横屏卡片 UI (Slate & Neon)
* **Page 0 (监测页)**：以 Slate 暗灰色作为卡片底色，配以精致的卡片灰色边框，并在横屏上采用两栏布局以提供大量监控数据（温、湿、压、AI 滤波、健康状态等）。
* **Page 1 (自适应页)**：左侧为 AI 基准数值卡片及对称偏差进度条，右侧为横展的 24 点示波器式温度偏差折线图。
* **Page 2 (日志页)**：以霓虹时间轴加右侧浮动卡片形式展示最近 4 条异常事件快照。

### 6.4 警笛交替 LED 指示灯
* 学习模式下 LED3 亮起，正常监控下 LED4 亮起。
* 处于异常状态（传感器掉线或温度异常报警）时，TIM2 呼吸灯自动熄灭，PC13 告警闪烁，LED3 和 LED4 自动进行 5Hz 高频红蓝交替闪烁，发挥强烈的声光报警作用。

### 6.5 高性能 Flappy Bird 像素游戏 (Page 3)
* **Dirty Rects 增量刷新**：针对 SPI 慢速刷屏的硬伤，我们只在 30ms 帧循环中擦除和重画移动像素（12×12 的黄色小鸟和 4 像素宽的管道变化边缘），让游戏能够在软件模拟 SPI 下流畅跑到 **30+ FPS**。
* **物理引擎与按键防抖缓存**：完整实现了重力加速度、飞跃拍击速度和精确的包围盒碰撞检测。按键采用 `game_key` 异步缓存机制，彻底解决 30ms 游戏循环漏检 10ms 消抖按键的 Bug，拥有极佳的跳跃灵敏度。

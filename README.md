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

## 5. 多 Agent 协作规范
如果您是 AI 辅助编码 Agent，请在开发前仔细阅读 [agent.md](file:///d:/MICROCOMPUTER/micro-com-final/agent.md)，了解头文件路径配置、工程文件修改规范以及代码编写标准。

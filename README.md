# STM32F103C8T6 LED Driver

基于 STM32F103C8T6（Blue Pill）的 LED 驱动程序，使用 Keil MDK + ARM Compiler 6 开发。

## 项目结构

```
├── Hardware/
│   ├── led.c          # LED 驱动实现
│   └── led.h          # LED 引脚配置与 API 头文件
├── User/
│   ├── main.c         # 主程序入口
│   └── main.h
├── RTE/               # Keil RTE 运行时环境
│   ├── Device/STM32F103C8/
│   └── _Target_1/
└── demo.uvprojx       # Keil MDK 工程文件
```

## 功能特性

- 支持 4 路 LED 灯控制（默认配置见下表）
- 表驱动架构，方便扩展 LED 数量
- 支持有源高/有源低电平配置
- 提供开/关/翻转/写入四种控制接口

## 默认引脚配置

| LED  | GPIO  | 引脚 | 有源电平 |
|------|-------|------|----------|
| LED1 | GPIOC | PC13 | 低电平有效 |
| LED2 | GPIOA | PA0  | 高电平有效 |
| LED3 | GPIOA | PA1  | 高电平有效 |
| LED4 | GPIOA | PA2  | 高电平有效 |

## API 接口

```c
void LED_Init(void);                          // 初始化所有 LED
void LED_On(uint8_t index);                   // 点亮指定 LED
void LED_Off(uint8_t index);                  // 关闭指定 LED
void LED_Toggle(uint8_t index);               // 翻转指定 LED
void LED_Write(uint8_t index, uint8_t state); // 写入 LED 状态
```

## 使用方法

1. 用 Keil MDK 打开 `demo.uvprojx` 工程文件
2. 在 `Hardware/led.h` 中修改引脚配置以匹配你的硬件接线
3. 在 `User/main.c` 中编写主逻辑
4. 编译并烧录到目标板

## 开发工具

- **IDE**: Keil MDK
- **编译器**: ARM Compiler 6
- **目标芯片**: STM32F103C8T6

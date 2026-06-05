# STM32 Keil5 AI Agent 协作与环境规范 (agent.md)

此文件用于向后续参与该项目的 AI Agent 提供标准化的开发环境说明、目录结构约定及协作指南，以确保多 Agent 协作时开发环境和代码标准的完全一致。

---

## 1. 运行与编译环境 (Development Environment)

在编译、修改或验证代码前，请确保遵循以下硬件与工具链配置：

* **目标芯片 (Target MCU)**: STM32F103C8T6 (Cortex-M3 内核，64 KB Flash，20 KB SRAM)
* **开发工具 (IDE)**: Keil uVision 5 (Keil5)
* **编译器 (Toolchain)**: **ARM Compiler 6 (AC6)** (而非 Arm Compiler 5，已在工程中启用 `<uAC6>1</uAC6>`)
* **编码格式 (Encoding)**: 所有源文件及头文件必须使用 **UTF-8** 编码，防止在不同系统或 Git 提交中出现中文乱码。
* **C 标准 (C Standard)**: C99 / GNU99 (`--c99` 兼容模式)。

---

## 2. 目录结构规范 (Project Directory Structure)

项目采用清晰的三层架构设计，所有新增文件必须归入以下指定目录：

```
d:\MICROCOMPUTER\micro-com-final\
├── User/               # 应用层：存放主程序及中断服务程序
│   ├── main.c          # 主程序入口
│   └── main.h          # 主程序头文件
├── Hardware/           # 硬件驱动层：存放外部传感器及外设驱动
│   ├── led.c           # LED 驱动实现
│   └── led.h           # LED 配置与 API
├── Library/            # 库文件层：存放 CMSIS 核心文件或第三方库文件
│   └── .gitkeep        # Git 占位文件
├── RTE/                # Keil RTE 运行时环境（启动文件及系统配置文件）
│   └── Device/STM32F103C8/
│       ├── startup_stm32f10x_md.s  # 启动文件
│       └── system_stm32f10x.c      # 系统时钟配置文件
└── demo.uvprojx        # Keil 5 工程主配置文件
```

---

## 3. Keil 工程配置约定 (Keil Project Settings)

为保证代码顺利编译，所有 Agent 在添加新文件时需确保 `demo.uvprojx` 满足以下配置：

### 3.1 头文件包含路径 (Include Paths)
在 C/C++ 选项卡中，头文件搜索路径必须包含以下三个相对路径：
* `.\User`
* `.\Hardware`
* `.\Library`
*(已在 `demo.uvprojx` 中配置为：`<IncludePath>.\User;.\Hardware;.\Library</IncludePath>`)*

### 3.2 工程组 (Project Groups)
Keil 中的 Group 命名与磁盘上的目录结构一一对应。新增 `.c` 文件时，必须同步修改 `demo.uvprojx` 的 `<Groups>` 段，将文件注册到对应的 Group 中：
* **User** 组：包含 `User/` 目录下的 `.c` 文件。
* **Hardware** 组：包含 `Hardware/` 目录下的 `.c` 文件。
* **Library** 组：包含 `Library/` 目录下的 `.c` 文件。

---

## 4. 驱动编写规范 (Driver Writing Conventions)

为保证底层硬件代码的健壮性与可读性：

1. **表驱动架构**：外设引脚配置推荐使用结构体描述，并使用静态数组查表初始化（参考 `led.c` 的实现）。
2. **寄存器操作**：在没有引入官方标准外设库前，直接通过 STM32 寄存器（如 `BSRR`, `BRR`, `ODR`, `CRL`, `CRH`）进行引脚控制。
3. **GPIO 电平翻转 (Toggle)**：翻转逻辑不得依赖逻辑状态（如 `active_level`），而应直接通过读取 `ODR` 电平，再配合 `BSRR` / `BRR` 写入相反的物理电平。例如：
   ```c
   if (led->port->ODR & led->pin) {
       led->port->BRR = led->pin;   // 原为高电平，拉低
   } else {
       led->port->BSRR = led->pin;  // 原为低电平，拉高
   }
   ```

---

## 5. Agent 提交流程与规范 (Agent Git Workflow)

1. **修改文件前**：阅读此 `agent.md` 和 `README.md`，确认没有环境冲突。
2. **修改 Keil 工程时**：若新增了源文件，请务必修改 `demo.uvprojx`。
3. **修改完成后**：先本地编译（如有编译环境），并使用 `git status` 确认修改范围。
4. **提交代码**：编写清晰的 Commit Message，指出修复的 BUG 或新增的功能，并推送到 GitHub。

---

## 6. 代码注释规范 (Code Commenting Conventions)

为方便课程设计评审和多成员团队的高效协作：

1. **全中文注释**：所有新增、修改的源文件 (`.c`) 和头文件 (`.h`) 里的代码注释必须全部使用**简体中文**撰写。禁止在物理驱动、算法计算、寄存器位设置、变量及宏定义旁使用英文注释。
2. **编码格式保护**：所有代码文件及 `agent.md` 等文档必须统一采用 **UTF-8** 编码保存，避免在不同开发机（如 Windows Keil5 或各类文本编辑器）或远端 Git 平台上拉取代码时出现中文字符集解码乱码问题。
3. **关键部分详释**：
   * 对寄存器位操作（如 GPIO 的 CRL/CRH、TIM2 的 CCMR1/CCER 等寄存器位配置）需在中文注释中清晰写明具体位值和配置逻辑。
   * 对自适应滤波（如 EMA 衰减系数 $\alpha$、平滑计算公式）和图形渲染（如折线图坐标轴位置、动态缩放比）需用中文详细写出参数背景与运算逻辑。

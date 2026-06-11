# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/index.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

new_manifest = '''window.DECK_MANIFEST = [
    { file: "slides/section0-01-cover.html",             label: "封面" },
    { file: "slides/section0-02-agenda.html",            label: "汇报目录与系统设计大纲" },
    { file: "slides/section1-01-divider-background.html", label: "01 项目背景与应用场景定位" },
    { file: "slides/section1-02-background.html",        label: "01 项目背景与智能家居系统架构" },
    { file: "slides/section2-01-divider-hardware.html",   label: "02 物理硬件底座与引脚重构" },
    { file: "slides/section2-02-hardware-layout.html",    label: "02 系统硬件组成与接口类型全览" },
    { file: "slides/section2-03-pin-reconfig.html",      label: "02 引脚冲突规避与物理重排重映射" },
    { file: "slides/section3-01-divider-driver.html",     label: "03 底层驱动协议与时序优化" },
    { file: "slides/section3-02-bus-overview.html",      label: "03 系统通信总线与传输协议规范" },
    { file: "slides/section3-03-protocol-drivers.html",  label: "03 底层驱动协议开发与深度寄存器优化" },
    { file: "slides/section4-01-divider-ui.html",        label: "04 HUD UI 设计与高保真仿真展示" },
    { file: "slides/section4-02-hud-pages-overview.html",label: "04 HUD 六大核心功能页面全览" },
    { file: "slides/section4-03-hud-ui-design.html",     label: "04 TFT-LCD 界面布局设计与双主题对比" },
    { file: "slides/section4-04-hmi-controls.html",      label: "04 多通道人机交互：实体/触屏/红外遥控" },
    { file: "slides/section5-01-divider-algorithm.html", label: "05 边缘端轻量级计算与算法优化" },
    { file: "slides/section5-02-edge-algorithms.html",   label: "05 边缘端轻量级计算特征提取算法" },
    { file: "slides/section5-03-dirty-rects.html",       label: "05 LCD 图形渲染优化与增量更新算法" },
    { file: "slides/section6-01-divider-security.html",  label: "06 多通道安全网关与物联网通信" },
    { file: "slides/section6-02-home-security.html",     label: "06 智能居家多级安全防护功能设计" },
    { file: "slides/section6-03-wechat-logging.html",    label: "06 云边端物联网通信与 SD 日志归档" },
    { file: "slides/section7-01-divider-testing.html",   label: "07 整机联合调试与系统总结" },
    { file: "slides/section7-02-joint-verification.html", label: "07 系统联合测试与运行指标验证" },
    { file: "slides/section7-03-summary.html",           label: "07 系统设计总结" },
    { file: "slides/section7-04-experience.html",        label: "07 系统设计经验与感悟分享" },
    { file: "slides/section8-01-thanks.html",            label: "08 谢幕致谢" }
  ];'''

c = re.sub(r'window\.DECK_MANIFEST\s*=\s*\[.*?\];', new_manifest, c, flags=re.DOTALL)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Updated DECK_MANIFEST completely")

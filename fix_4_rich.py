# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section1-02-background.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

import re

old_ul_match = re.search(r'<ul class="pain-list">.*?</ul>', c, re.DOTALL)
if old_ul_match:
    old_ul = old_ul_match.group(0)
    
    new_ul = '''<ul class="pain-list">
      <li><strong>系统定位</strong>：<br>
        本项目致力于打造一款综合型、高集成的“环境自适应自愈型智能居家安全网关”。<br>
        硬件底座以 STM32F103 微控制器为核心，深度集成多维物理传感器阵列（包括 AHT20 温湿度、BMP280 气压、BH1750 光照以及超声波测距）。<br>
        架构上横跨“物理感知、边缘计算、人机交互、底层驱动、云端通信”五大层次，集成了本地安全防御、离线异常日志归档存储，以及基于 ESP8266 WiFi 的物联网心跳维持与微信实时告警推送功能，构建出一个全天候、全方位的高可用智能家居中枢体系。
      </li>
      <li><strong>核心优化策略</strong>：<br>
        在软件算法与系统调度层面，彻底打破了传统嵌入式设备依赖死板静态阈值以及高耗能全屏刷新的设计瓶颈。<br>
        数据侧：创新性地引入了环境数据一阶 EMA（指数移动平均）低通滤波去噪，以及长达 150s 的环境基准自学习判定模型，极大地降低了环境噪声带来的误报率。<br>
        渲染侧：自主编写并应用了屏幕增量脏矩形（Dirty Rectangle）局部刷新算法，有效规避了 SPI 宽带瓶颈，实现了堪比智能手机的丝滑高帧率 UI 动画体验。<br>
        通信侧：构建了具有状态机隔离机制的网络自愈调度策略，并加入了 180s 物理冷却防抖软锁，确保设备在弱网环境乃至断网重连下依然具备极高的系统鲁棒性与零挂死率。
      </li>
    </ul>'''
    
    c = c.replace(old_ul, new_ul)
    
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(c)
    print("SUCCESS: Expanded content on page 4")
else:
    print("WARNING: ul element not found")

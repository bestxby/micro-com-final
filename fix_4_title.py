# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section1-02-background.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Update titles
c = c.replace('<h1 class="slide-title">研究背景与系统设计定位</h1>', '<h1 class="slide-title">系统设计定位与核心优化策略</h1>')
c = c.replace('<p class="slide-subtitle">Project Background & System Architectural Positioning</p>', '<p class="slide-subtitle">System Design Positioning and Core Optimization Strategies</p>')

# 2. Add bold back to "系统定位" and "优化策略"
c = c.replace('<li>系统定位：', '<li><strong>系统定位</strong>：')
c = c.replace('<li>优化策略：', '<li><strong>优化策略</strong>：')

# 3. Update footer section name if it says "背景"
c = c.replace('<div class="section-name">01. 项目背景与应用场景定位</div>', '<div class="section-name">01. 系统架构与定位</div>')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Updated title and bolded keywords on page 4")

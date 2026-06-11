# -*- coding: utf-8 -*-
import os, glob

# 1. Update Slide 16 (Make remote smaller)
controls_path = glob.glob('*文档/slides/section5-04-hmi-controls.html')[0]
with open(controls_path, 'r', encoding='utf-8') as f:
    controls_content = f.read()

# Change scale
controls_content = controls_content.replace('transform: scale(0.70);', 'transform: scale(0.58);')
controls_content = controls_content.replace('transform: scale(0.65);', 'transform: scale(0.58);') # Just in case

with open(controls_path, 'w', encoding='utf-8') as f:
    f.write(controls_content)

# 2. Update Slide 15 (Add play video button to bottom right)
overview_path = glob.glob('*文档/slides/section5-03-hud-pages-overview.html')[0]
with open(overview_path, 'r', encoding='utf-8') as f:
    overview_content = f.read()

btn_html = """
<button style="position: absolute; bottom: 80px; right: 50px; z-index: 1000; padding: 16px 32px; background: #0F766E; color: white; border: none; border-radius: 12px; font-size: 20px; font-weight: bold; cursor: pointer; box-shadow: 0 8px 25px rgba(15,118,110,0.4); transition: all 0.2s; display: flex; align-items: center; gap: 8px;" onmouseover="this.style.background='#115E59'; this.style.transform='scale(1.05) translateY(-3px)';" onmouseout="this.style.background='#0F766E'; this.style.transform='none';">
  <span style="font-size: 24px;">▶</span> 播放核心功能综合演示
</button>

<div class="slide-footer">
"""

overview_content = overview_content.replace('<div class="slide-footer">', btn_html)

with open(overview_path, 'w', encoding='utf-8') as f:
    f.write(overview_content)

print("SUCCESS")

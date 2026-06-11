# -*- coding: utf-8 -*-
import os, glob, re

path = glob.glob('*文档/slides/section4-03-hud-ui-design.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

# Replace Theme 0 and Theme 1 tags
content = content.replace('主题 0：Dark 黑色霓虹模式', '主题 1：Neon Black 极客霓虹')
content = content.replace('主题 1：Deep Space Blue 深海暗蓝', '主题 2：Dark Blue 深空暗蓝')

# Create the new left-intro HTML
new_intro = """      <h2 style="font-size: 32px; font-weight: 800; color: var(--text-primary); margin-bottom: 24px; border-left: 5px solid var(--brand-secondary); padding-left: 16px;">界面布局与视觉引擎</h2>
      <p>
        本项目在 480x320 分辨率的 TFT-LCD 上，通过纯底层 C 语言绘制逻辑，从零搭建了一套极具科技感的类 HUD (平视显示器) 高帧率交互系统。系统内嵌两套深度定制的视觉主题，以完美适应不同的应用场景与环境光线。
      </p>
      
      <ul class="design-points">
        <li><strong>模块化卡片架构 (Card-UI)</strong>：采用现代化的扁平悬浮设计语言，将温湿度、环境光照、安全评级等关键数据进行区块化精准隔离，信息层级分明，一目了然。</li>
        <li><strong>双轨动态色彩映射</strong>：底层抽象出全局色彩配置指针，支持一键在“极客霓虹”与“深空暗蓝”两套高对比度主题间无缝热切换。</li>
        <li><strong>像素级交互状态反馈</strong>：顶部配备全局安全屏障状态指示栏，配合底部的实时微秒级运行时钟与动态页码指示器，打造媲美现代智能终端的流畅体验。</li>
      </ul>
"""

# Replace the inner HTML of left-intro
start_m = re.search(r'<div class="left-intro">', content)
if start_m:
    start_idx = start_m.end()
    end_idx = content.find('<div class="right-screens">', start_idx)
    
    # Actually end_idx is right after the closing </div> of left-intro.
    # Let's find the closing </div> of left-intro precisely.
    # It ends right before <!-- Right panel: Side by Side Screen Mockups -->
    real_end = content.find('</div>\n\n    <!-- Right panel:')
    
    if real_end != -1:
        content = content[:start_idx] + "\n" + new_intro + "    " + content[real_end:]

with open(path, 'w', encoding='utf-8') as f:
    f.write(content)

print("SUCCESS")

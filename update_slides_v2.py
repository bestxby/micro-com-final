# -*- coding: utf-8 -*-
import os
import re
import glob

index_path = glob.glob('*文档/index.html')
slides_dir = os.path.join(os.path.dirname(index_path[0]), 'slides')

overview_path = os.path.join(slides_dir, 'section5-03-hud-pages-overview.html')
controls_path = os.path.join(slides_dir, 'section5-04-hmi-controls.html')

# 1. Update the scale in overview
with open(overview_path, 'r', encoding='utf-8') as f:
    overview_content = f.read()

# Replace scale(0.48) with scale(0.85)
overview_content = overview_content.replace('transform: scale(0.48)', 'transform: scale(0.85)')
with open(overview_path, 'w', encoding='utf-8') as f:
    f.write(overview_content)

# 2. Re-create controls slide with remote CSS/HTML and buttons
# First get the CSS and HTML for the remote from temp_slide.txt
with open('temp_slide.txt', 'r', encoding='utf-8') as f:
    temp_content = f.read()

# Extract remote CSS
css_match = re.search(r'(/\* Virtual IR Remote Styling \*/.*?)(\/\* LCD Screen Emulator Enclosure \*/)', temp_content, re.DOTALL)
if css_match:
    remote_css = css_match.group(1)
else:
    remote_css = ""

start = temp_content.find('<div class="remote-control">')
end = temp_content.find('<!-- Right:', start)
remote_html = temp_content[start:end].strip()

# Modify the remote HTML to remove the text below the buttons if it exists
text_start = remote_html.find('<div style="font-size: 15px;')
if text_start != -1:
    text_end = remote_html.find('</div>', text_start) + 6
    remote_html = remote_html[:text_start] + remote_html[text_end:]

with open(controls_path, 'r', encoding='utf-8') as f:
    controls_content = f.read()

# Remove old CSS if there was one (just in case), otherwise inject
if '/* Virtual IR Remote Styling */' not in controls_content:
    controls_content = controls_content.replace('</head>', f'<style>\n{remote_css}\n</style>\n</head>')

video_btn_style = "margin-top: 20px; padding: 12px 24px; background: #0F766E; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; box-shadow: 0 4px 12px rgba(15,118,110,0.3); transition: all 0.2s;"
video_btn_hover = "onmouseover=\"this.style.background='#115E59'; this.style.transform='translateY(-2px)'\" onmouseout=\"this.style.background='#0F766E'; this.style.transform='none'\""

new_cols_html = f"""  <div style="display: flex; justify-content: center; align-items: stretch; height: 100%; padding: 40px 60px; gap: 40px;">
    
    <!-- 1. 实体按键 -->
    <div style="flex: 1; background: #FCFAF7; border: 1px solid #EAE6DF; border-radius: 20px; padding: 40px; text-align: center; box-shadow: 0 15px 35px rgba(25,24,22,0.08); display: flex; flex-direction: column;">
      <div style="height: 160px; display: flex; align-items: center; justify-content: center; font-size: 80px;">🕹️</div>
      <h2 style="color: var(--brand-primary); margin-bottom: 24px; font-size: 28px;">1. 实验箱物理按键</h2>
      <p style="color: var(--text-muted); font-size: 18px; line-height: 1.8; text-align: left; background: #F3EBE1; padding: 20px; border-radius: 12px; flex-grow: 1;">
        <strong style="color: #4A453E;">输入源：</strong><br>实验板载独立按键 (KEY1-KEY4) + 五向导航摇杆<br><br>
        <strong style="color: #4A453E;">特点：</strong><br>
        提供最稳定、零延迟的硬件级控制，带有物理段落感。<br>适用于基础的菜单切换、系统急停复位及游戏控制。
      </p>
      <button style="{video_btn_style}" {video_btn_hover}>▶ 播放按键操控演示</button>
    </div>

    <!-- 2. 触摸屏 -->
    <div style="flex: 1; background: #FCFAF7; border: 1px solid #EAE6DF; border-radius: 20px; padding: 40px; text-align: center; box-shadow: 0 15px 35px rgba(25,24,22,0.08); display: flex; flex-direction: column;">
      <div style="height: 160px; display: flex; align-items: center; justify-content: center; font-size: 80px;">👆</div>
      <h2 style="color: var(--brand-primary); margin-bottom: 24px; font-size: 28px;">2. TFT 电阻触摸屏</h2>
      <p style="color: var(--text-muted); font-size: 18px; line-height: 1.8; text-align: left; background: #F3EBE1; padding: 20px; border-radius: 12px; flex-grow: 1;">
        <strong style="color: #4A453E;">输入源：</strong><br>XPT2046 触摸控制器采集压感坐标<br><br>
        <strong style="color: #4A453E;">特点：</strong><br>
        实现直观的图形化 GUI 交互。用户可直接点击屏幕虚拟按钮或进行交互操作，体验类似现代智能手机的直觉控制。
      </p>
      <button style="{video_btn_style}" {video_btn_hover}>▶ 播放触屏操控演示</button>
    </div>

    <!-- 3. 红外遥控 -->
    <div style="flex: 1; background: #FCFAF7; border: 1px solid #EAE6DF; border-radius: 20px; padding: 40px; text-align: center; box-shadow: 0 15px 35px rgba(25,24,22,0.08); display: flex; flex-direction: column;">
      <div style="height: 250px; display: flex; align-items: center; justify-content: center; transform: scale(0.65); transform-origin: center center; margin-top: -30px; margin-bottom: -10px;">
        {remote_html}
      </div>
      <h2 style="color: var(--brand-primary); margin-bottom: 24px; font-size: 28px;">3. NEC 红外遥控器</h2>
      <p style="color: var(--text-muted); font-size: 18px; line-height: 1.8; text-align: left; background: #F3EBE1; padding: 20px; border-radius: 12px; flex-grow: 1;">
        <strong style="color: #4A453E;">输入源：</strong><br>17 键红外遥控器 + EXTI 外部中断 + TIM4<br><br>
        <strong style="color: #4A453E;">特点：</strong><br>
        实现远距离无线操控。支持翻页、布撤防、主题切换等高频操作。采用标准 NEC 协议严格解码，抗干扰性强。
      </p>
      <button style="{video_btn_style}" {video_btn_hover}>▶ 播放红外遥控演示</button>
    </div>

  </div>"""

start_idx = controls_content.find('<div style="display: flex; justify-content: center; align-items: stretch;')
end_idx = controls_content.find('</div>\n</div>\n\n<div class="slide-footer">') + 6
if start_idx != -1 and end_idx != -1:
    controls_content = controls_content[:start_idx] + new_cols_html + controls_content[end_idx:]

with open(controls_path, 'w', encoding='utf-8') as f:
    f.write(controls_content)

print("SUCCESS")

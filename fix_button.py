# -*- coding: utf-8 -*-
import os, glob

# Update Slide 15 (Fix button position and size)
overview_path = glob.glob('*文档/slides/section5-03-hud-pages-overview.html')[0]
with open(overview_path, 'r', encoding='utf-8') as f:
    overview_content = f.read()

# The old button HTML
# <button style="position: absolute; bottom: 80px; right: 50px; z-index: 1000; padding: 16px 32px; background: #0F766E; color: white; border: none; border-radius: 12px; font-size: 20px; font-weight: bold; cursor: pointer; box-shadow: 0 8px 25px rgba(15,118,110,0.4); transition: all 0.2s; display: flex; align-items: center; gap: 8px;" onmouseover="this.style.background='#115E59'; this.style.transform='scale(1.05) translateY(-3px)';" onmouseout="this.style.background='#0F766E'; this.style.transform='none';">
#   <span style="font-size: 24px;">▶</span> 播放核心功能综合演示
# </button>

# Find it and replace it.
start_idx = overview_content.find('<button style="position: absolute; bottom: 80px; right: 50px;')
if start_idx != -1:
    end_idx = overview_content.find('</button>', start_idx) + 9
    
    # New button HTML
    # Placed safely at the top right, avoiding the 15% right overlay (1920 * 0.15 = 288px)
    new_btn = """<button style="position: absolute; top: 35px; right: 320px; z-index: 1000; padding: 10px 20px; background: #0F766E; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; box-shadow: 0 4px 15px rgba(15,118,110,0.3); transition: all 0.2s; display: flex; align-items: center; gap: 6px;" onmouseover="this.style.background='#115E59'; this.style.transform='translateY(-2px)';" onmouseout="this.style.background='#0F766E'; this.style.transform='none';">
  <span style="font-size: 18px;">▶</span> 播放综合演示视频
</button>"""
    
    overview_content = overview_content[:start_idx] + new_btn + overview_content[end_idx:]
    
    with open(overview_path, 'w', encoding='utf-8') as f:
        f.write(overview_content)
    print("SUCCESS")
else:
    print("COULD NOT FIND BUTTON")

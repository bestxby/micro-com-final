# -*- coding: utf-8 -*-
import os, glob, re

slides_dir = glob.glob('*文档/slides')[0]

btn_html = """<button class="view-video-btn">
  <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
    <circle cx="12" cy="12" r="10"></circle>
    <polygon points="10 8 16 12 10 16 10 8"></polygon>
  </svg>
  查看视频演示
</button>
"""

# 1. Remove the huge custom button from Slide 12
slide12_path = os.path.join(slides_dir, 'section4-02-hud-pages-overview.html')
with open(slide12_path, 'r', encoding='utf-8') as f:
    c12 = f.read()

# The button starts with <button style="position: absolute; bottom: 80px; right: 50px;
start_idx = c12.find('<button style="position: absolute; bottom: 80px; right: 50px;')
if start_idx != -1:
    end_idx = c12.find('</button>', start_idx) + 9
    c12 = c12[:start_idx] + c12[end_idx:]
    with open(slide12_path, 'w', encoding='utf-8') as f:
        f.write(c12)

# 2. Add the view-video-btn to Slides 13, 14, 16, 17
targets = [
    'section4-03-hud-ui-design.html', # Slide 13
    'section4-04-hmi-controls.html',  # Slide 14
    'section5-02-edge-algorithms.html', # Slide 16
    'section5-03-dirty-rects.html'    # Slide 17
]

for t in targets:
    p = os.path.join(slides_dir, t)
    if os.path.exists(p):
        with open(p, 'r', encoding='utf-8') as f:
            c = f.read()
        if 'view-video-btn' not in c:
            c = c.replace('<div class="slide-footer">', btn_html + '<div class="slide-footer">')
            with open(p, 'w', encoding='utf-8') as f:
                f.write(c)

print("SUCCESS")

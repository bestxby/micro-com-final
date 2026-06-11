# -*- coding: utf-8 -*-
import os, glob

# 1. Update tokens.css
tokens_path = glob.glob('*文档/shared/tokens.css')[0]
with open(tokens_path, 'r', encoding='utf-8') as f:
    css = f.read()

if '.view-video-btn {' in css:
    start_btn = css.find('.view-video-btn {')
    end_btn = css.find('}', start_btn)
    btn_block = css[start_btn:end_btn]
    new_btn_block = btn_block.replace('bottom: 30px;', 'bottom: 80px;')
    css = css[:start_btn] + new_btn_block + css[end_btn:]
    with open(tokens_path, 'w', encoding='utf-8') as f:
        f.write(css)

# 2. Delete "语言" from Slide 13
slide13_path = glob.glob('*文档/slides/section4-03-hud-ui-design.html')[0]
with open(slide13_path, 'r', encoding='utf-8') as f:
    c13 = f.read()
c13 = c13.replace('扁平悬浮设计语言，', '扁平悬浮设计，')

# 3. Replace the button HTML in 13, 16, 17
new_btn = """<button class="view-video-btn" id="openModalBtn">
    <span class="play-icon">▶</span> 查看视频演示
  </button>"""

def replace_btn(content):
    start = content.find('<button class="view-video-btn">')
    if start != -1:
        end = content.find('</button>', start) + 9
        content = content[:start] + new_btn + content[end:]
    return content

c13 = replace_btn(c13)
with open(slide13_path, 'w', encoding='utf-8') as f:
    f.write(c13)

for t in ['section5-02-edge-algorithms.html', 'section5-03-dirty-rects.html']:
    paths = glob.glob('*文档/slides/' + t)
    if paths:
        p = paths[0]
        with open(p, 'r', encoding='utf-8') as f:
            c = f.read()
        c = replace_btn(c)
        with open(p, 'w', encoding='utf-8') as f:
            f.write(c)

print("SUCCESS")

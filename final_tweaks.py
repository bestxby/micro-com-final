# -*- coding: utf-8 -*-
import os, glob

base_dir = glob.glob('*文档')[0]

# 1. Update tokens.css to move the button up to bottom: 80px
tokens_path = os.path.join(base_dir, 'shared', 'tokens.css')
with open(tokens_path, 'r', encoding='utf-8') as f:
    css = f.read()
# Replace bottom: 30px; with bottom: 80px; inside .view-video-btn
if '.view-video-btn {' in css:
    start_btn = css.find('.view-video-btn {')
    end_btn = css.find('}', start_btn)
    btn_block = css[start_btn:end_btn]
    new_btn_block = btn_block.replace('bottom: 30px;', 'bottom: 80px;')
    css = css[:start_btn] + new_btn_block + css[end_btn:]
    with open(tokens_path, 'w', encoding='utf-8') as f:
        f.write(css)

# 2. Delete "语言" from Slide 13
slide13_path = os.path.join(base_dir, 'slides', 'section4-03-hud-ui-design.html')
with open(slide13_path, 'r', encoding='utf-8') as f:
    c13 = f.read()
c13 = c13.replace('扁平悬浮设计语言，', '扁平悬浮设计，')

# 3. Replace the button HTML in 13, 16, 17 with the exact one from Slide 19/20
old_btn = """<button class="view-video-btn">
  <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
    <circle cx="12" cy="12" r="10"></circle>
    <polygon points="10 8 16 12 10 16 10 8"></polygon>
  </svg>
  查看视频演示
</button>"""

new_btn = """<button class="view-video-btn" id="openModalBtn">
    <span class="play-icon">▶</span> 查看视频演示
  </button>"""

def replace_btn(content):
    # Depending on exact whitespace, we might just search for <button class="view-video-btn"> and remove up to </button>
    start = content.find('<button class="view-video-btn">')
    if start != -1:
        end = content.find('</button>', start) + 9
        content = content[:start] + new_btn + content[end:]
    return content

c13 = replace_btn(c13)
with open(slide13_path, 'w', encoding='utf-8') as f:
    f.write(c13)

for t in ['section5-02-edge-algorithms.html', 'section5-03-dirty-rects.html']:
    p = os.path.join(base_dir, 'slides', t)
    if os.path.exists(p):
        with open(p, 'r', encoding='utf-8') as f:
            c = f.read()
        c = replace_btn(c)
        with open(p, 'w', encoding='utf-8') as f:
            f.write(c)

print("SUCCESS")

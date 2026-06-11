# -*- coding: utf-8 -*-
import os, glob, re, shutil

index_paths = glob.glob('展示文档/index.html')
if not index_paths:
    # try picking the correct one if encoding makes it weird
    for p in glob.glob('*/index.html'):
        if '展示' in p or '文档' in p and '参考' not in p:
            index_path = p
            break
else:
    index_path = index_paths[0]

base_dir = os.path.dirname(index_path)
slides_dir = os.path.join(base_dir, 'slides')

# 1. Rename dirty rects
old_dirty = os.path.join(slides_dir, 'section5-05-dirty-rects.html')
new_dirty = os.path.join(slides_dir, 'section4-03-dirty-rects.html')
if os.path.exists(old_dirty):
    shutil.move(old_dirty, new_dirty)

# 2. Update index.html manifest
with open(index_path, 'r', encoding='utf-8') as f:
    idx_content = f.read()

# We need to reorder the javascript manifest. It's easiest to just replace the block of 5 items.
# Let's extract the array
start = idx_content.find('window.DECK_MANIFEST = [')
end = idx_content.find('];', start)
manifest_text = idx_content[start:end]

# We know the old lines.
old_lines = [
    '{ file: "slides/section5-01-divider-ui.html",          label: "13 [页] 05. HUD UI 设计与高保真仿真展示" },',
    '{ file: "slides/section5-01-divider-ui.html",          label: "14 [页] 05. HUD UI 设计与高保真仿真展示" },',
    '{ file: "slides/section5-02-hud-ui-design.html",     label: "14 TFT-LCD 界面布局设计与仿真展示" },',
    '{ file: "slides/section5-03-hud-pages-overview.html",label: "15 HUD 六大核心功能页面全览" },',
    '{ file: "slides/section5-04-hmi-controls.html",      label: "16 多通道人机交互：实体/触屏/红外遥控" },',
    '{ file: "slides/section5-05-dirty-rects.html",       label: "17 LCD 图形渲染优化与增量更新算法" },'
]

new_lines = [
    '{ file: "slides/section4-03-dirty-rects.html",       label: "13 LCD 图形渲染优化与增量更新算法" },',
    '{ file: "slides/section5-01-divider-ui.html",          label: "14 [页] 05. HUD UI 设计与高保真仿真展示" },',
    '{ file: "slides/section5-03-hud-pages-overview.html",label: "15 HUD 六大核心功能页面全览" },',
    '{ file: "slides/section5-02-hud-ui-design.html",     label: "16 TFT-LCD 界面布局设计与双主题对比" },',
    '{ file: "slides/section5-04-hmi-controls.html",      label: "17 多通道人机交互：实体/触屏/红外遥控" },'
]

for old_line in old_lines:
    idx_content = idx_content.replace(old_line, "")

# Now find where section4 ends and insert new_lines
insert_pos = idx_content.find('{ file: "slides/section4-02-edge-algorithms.html",   label: "12 边缘端轻量级计算特征提取算法" },')
if insert_pos == -1:
    insert_pos = idx_content.find('section4-02-edge-algorithms.html')
insert_pos = idx_content.find('\n', insert_pos) + 1

idx_content = idx_content[:insert_pos] + "    " + "\n    ".join(new_lines) + "\n" + idx_content[insert_pos:]
idx_content = os.linesep.join([s for s in idx_content.splitlines() if s.strip()])

with open(index_path, 'w', encoding='utf-8') as f:
    f.write(idx_content)

# 3. Update individual slide files footers and names

def update_footer(path, new_num, new_section=None):
    if not os.path.exists(path): return
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    content = re.sub(r'Page \d+ / \d+', f'Page {new_num} / 25', content)
    if new_section:
        content = re.sub(r'<div class="section-name">.*?</div>', f'<div class="section-name">{new_section}</div>', content)
    
    # Also update titles
    title_match = re.search(r'<title>\d+ (.*?)</title>', content)
    if title_match:
        content = re.sub(r'<title>\d+ (.*?)</title>', f'<title>{new_num:02d} {title_match.group(1)}</title>', content)
    
    # Check for subtitle changes in footers
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

update_footer(new_dirty, 13, "04. 环境感知与算法调度")
update_footer(os.path.join(slides_dir, 'section5-01-divider-ui.html'), 14)
update_footer(os.path.join(slides_dir, 'section5-03-hud-pages-overview.html'), 15)
update_footer(os.path.join(slides_dir, 'section5-02-hud-ui-design.html'), 16)
update_footer(os.path.join(slides_dir, 'section5-04-hmi-controls.html'), 17)

# 4. Increase font size in section5-02-hud-ui-design.html
design_path = os.path.join(slides_dir, 'section5-02-hud-ui-design.html')
with open(design_path, 'r', encoding='utf-8') as f:
    design_content = f.read()

style_inject = """
  .left-col p { font-size: 21px; line-height: 1.8; }
  .left-col li { font-size: 22px; line-height: 1.8; }
  .left-col strong { font-size: 25px; }
"""
if '.left-col p {' not in design_content:
    design_content = design_content.replace('</style>', style_inject + '\n</style>')

with open(design_path, 'w', encoding='utf-8') as f:
    f.write(design_content)

print("SUCCESS")

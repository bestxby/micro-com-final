# -*- coding: utf-8 -*-
import os, glob, re, shutil

index_paths = glob.glob('*文档/index.html')
index_path = index_paths[0]
slides_dir = os.path.dirname(index_path) + '/slides'

# 1. Rename files
def rename_if_exists(old, new):
    if os.path.exists(os.path.join(slides_dir, old)):
        shutil.move(os.path.join(slides_dir, old), os.path.join(slides_dir, new))

# Move section4 to temp
rename_if_exists('section4-01-divider-algorithm.html', 'temp-5-01-divider-algorithm.html')
rename_if_exists('section4-02-edge-algorithms.html', 'temp-5-02-edge-algorithms.html')
rename_if_exists('section4-03-dirty-rects.html', 'temp-5-03-dirty-rects.html')

# Move section5 to section4
rename_if_exists('section5-01-divider-ui.html', 'section4-01-divider-ui.html')
rename_if_exists('section5-03-hud-pages-overview.html', 'section4-02-hud-pages-overview.html')
rename_if_exists('section5-02-hud-ui-design.html', 'section4-03-hud-ui-design.html')
rename_if_exists('section5-04-hmi-controls.html', 'section4-04-hmi-controls.html')

# Move temp to section5
rename_if_exists('temp-5-01-divider-algorithm.html', 'section5-01-divider-algorithm.html')
rename_if_exists('temp-5-02-edge-algorithms.html', 'section5-02-edge-algorithms.html')
rename_if_exists('temp-5-03-dirty-rects.html', 'section5-03-dirty-rects.html')

# 2. Update index.html
with open(index_path, 'r', encoding='utf-8') as f:
    idx_content = f.read()

# Replace the specific block of section4 and section5
start = idx_content.find('{ file: "slides/section4-01-divider-algorithm.html"')
if start == -1:
    start = idx_content.find('{ file: "slides/section4-02-edge-algorithms.html"')

end = idx_content.find('{ file: "slides/section6-01-divider-security.html"')

if start != -1 and end != -1:
    new_manifest = """    { file: "slides/section4-01-divider-ui.html",          label: "11 [页] 04. HUD UI 设计与高保真仿真展示" },
    { file: "slides/section4-02-hud-pages-overview.html",label: "12 HUD 六大核心功能页面全览" },
    { file: "slides/section4-03-hud-ui-design.html",     label: "13 TFT-LCD 界面布局设计与双主题对比" },
    { file: "slides/section4-04-hmi-controls.html",      label: "14 多通道人机交互：实体/触屏/红外遥控" },
    { file: "slides/section5-01-divider-algorithm.html",   label: "15 [分] 05. 边缘端轻量级计算与算法优化" },
    { file: "slides/section5-02-edge-algorithms.html",   label: "16 边缘端轻量级计算特征提取算法" },
    { file: "slides/section5-03-dirty-rects.html",       label: "17 LCD 图形渲染优化与增量更新算法" },
"""
    idx_content = idx_content[:start] + new_manifest + idx_content[end:]

with open(index_path, 'w', encoding='utf-8') as f:
    f.write(idx_content)

# 3. Update Agenda
agenda_path = os.path.join(slides_dir, 'section0-02-agenda.html')
if os.path.exists(agenda_path):
    with open(agenda_path, 'r', encoding='utf-8') as f:
        agenda = f.read()
    
    agenda = agenda.replace('<div class="agenda-title">边缘端计算算法</div>', 'TEMP_ALGO_TITLE')
    agenda = agenda.replace('<div class="agenda-desc">EMA、自适应阈值与边缘算法</div>', 'TEMP_ALGO_DESC')
    # fallback to original
    agenda = agenda.replace('<div class="agenda-title">边缘端算法</div>', 'TEMP_ALGO_TITLE')
    agenda = agenda.replace('<div class="agenda-desc">EMA滤波、边缘自学习逻辑</div>', 'TEMP_ALGO_DESC')
    agenda = agenda.replace('<div class="agenda-title">边缘端计算与算法</div>', 'TEMP_ALGO_TITLE')
    agenda = agenda.replace('<div class="agenda-desc">EMA及自学习与图形渲染算法</div>', 'TEMP_ALGO_DESC')
    
    # old 5
    agenda = agenda.replace('<div class="agenda-title">HUD 渲染与界面</div>', 'TEMP_HUD_TITLE')
    agenda = agenda.replace('<div class="agenda-desc">双主题TFT-LCD、多通道人机交互</div>', 'TEMP_HUD_DESC')
    # fallback
    agenda = agenda.replace('<div class="agenda-title">HUD 界面渲染</div>', 'TEMP_HUD_TITLE')
    agenda = agenda.replace('<div class="agenda-desc">双主题TFT-LCD、人机交互</div>', 'TEMP_HUD_DESC')
    
    # Try regex if previous failed
    import re
    # We find agenda card 4 and 5
    card4_pattern = r'(<div class="agenda-num">04</div>\s*<div class="agenda-title">)(.*?)(</div>\s*<div class="agenda-desc">)(.*?)(</div>)'
    card5_pattern = r'(<div class="agenda-num">05</div>\s*<div class="agenda-title">)(.*?)(</div>\s*<div class="agenda-desc">)(.*?)(</div>)'
    
    agenda = re.sub(card4_pattern, r'\g<1>HUD UI 设计与人机交互\g<3>六大核心大屏与多通道控制\g<5>', agenda)
    agenda = re.sub(card5_pattern, r'\g<1>边缘计算与算法优化\g<3>AI 自学习、EMA 滤波与图形增量算法\g<5>', agenda)

    with open(agenda_path, 'w', encoding='utf-8') as f:
        f.write(agenda)

# 4. Update footers and titles in individual files
def update_slide(filename, new_page, new_section):
    p = os.path.join(slides_dir, filename)
    if not os.path.exists(p): return
    with open(p, 'r', encoding='utf-8') as f:
        c = f.read()
    
    # Title
    c = re.sub(r'<title>\d+ (.*?)</title>', f'<title>{new_page:02d} \g<1></title>', c)
    # Footer Section
    c = re.sub(r'<div class="section-name">.*?</div>', f'<div class="section-name">{new_section}</div>', c)
    # Footer Page Number
    c = re.sub(r'Page \d+ / \d+', f'Page {new_page} / 25', c)
    
    with open(p, 'w', encoding='utf-8') as f:
        f.write(c)

update_slide('section4-01-divider-ui.html', 11, '04. HUD UI 设计与人机交互')
update_slide('section4-02-hud-pages-overview.html', 12, '04. HUD UI 设计与人机交互')
update_slide('section4-03-hud-ui-design.html', 13, '04. HUD UI 设计与人机交互')
update_slide('section4-04-hmi-controls.html', 14, '04. HUD UI 设计与人机交互')

update_slide('section5-01-divider-algorithm.html', 15, '05. 边缘端计算与算法优化')
update_slide('section5-02-edge-algorithms.html', 16, '05. 边缘端计算与算法优化')
update_slide('section5-03-dirty-rects.html', 17, '05. 边缘端计算与算法优化')

# Fix massive numbers in dividers
def fix_divider(filename, num):
    p = os.path.join(slides_dir, filename)
    if not os.path.exists(p): return
    with open(p, 'r', encoding='utf-8') as f:
        c = f.read()
    c = re.sub(r'<div class="chapter-number">\d+</div>', f'<div class="chapter-number">{num:02d}</div>', c)
    with open(p, 'w', encoding='utf-8') as f:
        f.write(c)

fix_divider('section4-01-divider-ui.html', 4)
fix_divider('section5-01-divider-algorithm.html', 5)

print("SUCCESS")

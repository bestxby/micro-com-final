# -*- coding: utf-8 -*-
import os, glob

base_dir = [d for d in glob.glob('*文档') if '展示' in d][0]
slides_dir = os.path.join(base_dir, 'slides')

def fix_topic_list(content):
    # Swap the texts for 04 and 05 in the topic list.
    # Currently:
    # 04 ... 边缘数据计算与智能算法
    # 05 ... HUD 界面布局与增量渲染
    content = content.replace(
        '<div class="topic-num">04</div>\n          <div class="topic-text">边缘数据计算与智能算法</div>',
        '<div class="topic-num">04</div>\n          <div class="topic-text">HUD 界面布局与增量渲染</div>'
    )
    content = content.replace(
        '<div class="topic-num">05</div>\n          <div class="topic-text">HUD 界面布局与增量渲染</div>',
        '<div class="topic-num">05</div>\n          <div class="topic-text">边缘数据计算与智能算法</div>'
    )
    # Handle the case where one of them is active-topic (if the HTML structure was a bit different)
    # The active class is on the parent div, so the inner HTML is the same.
    return content

# Fix all divider slides
for div_file in glob.glob(os.path.join(slides_dir, 'section*-01-divider-*.html')):
    with open(div_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Fix the global topic list order
    content = fix_topic_list(content)
    
    # Fix the left part of section4
    if 'section4-01-divider-ui.html' in div_file:
        content = content.replace('<div class="part-num">PART 05</div>', '<div class="part-num">PART 04</div>')
        content = content.replace('<div class="huge-number">05</div>', '<div class="huge-number">04</div>')
        # Fix the title if it says 11 HUD ... -> it already says 11 HUD (which is page 11).
        
    # Fix the left part of section5
    if 'section5-01-divider-algorithm.html' in div_file:
        content = content.replace('<div class="part-num">PART 04</div>', '<div class="part-num">PART 05</div>')
        content = content.replace('<div class="huge-number">04</div>', '<div class="huge-number">05</div>')

    with open(div_file, 'w', encoding='utf-8') as f:
        f.write(content)

# Fix Agenda
agenda_path = os.path.join(slides_dir, 'section0-02-agenda.html')
with open(agenda_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Replace the text blocks in Agenda
# Agenda might use different classes, like <span class="ag-num">04</span>
content = content.replace(
    '<span class="ag-num">04</span><span class="ag-text">边缘数据计算与智能算法</span>',
    '<span class="ag-num">04</span><span class="ag-text">HUD 界面布局与增量渲染</span>'
)
content = content.replace(
    '<span class="ag-num">05</span><span class="ag-text">HUD 界面布局与增量渲染</span>',
    '<span class="ag-num">05</span><span class="ag-text">边缘数据计算与智能算法</span>'
)
with open(agenda_path, 'w', encoding='utf-8') as f:
    f.write(content)

print("SUCCESS")

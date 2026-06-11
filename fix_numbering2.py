# -*- coding: utf-8 -*-
import glob

def fix_topic_list(content):
    content = content.replace(
        '<div class="topic-num">04</div>\n          <div class="topic-text">边缘数据计算与智能算法</div>',
        '<div class="topic-num">04</div>\n          <div class="topic-text">HUD 界面布局与增量渲染</div>'
    )
    content = content.replace(
        '<div class="topic-num">05</div>\n          <div class="topic-text">HUD 界面布局与增量渲染</div>',
        '<div class="topic-num">05</div>\n          <div class="topic-text">边缘数据计算与智能算法</div>'
    )
    return content

for div_file in glob.glob('*文档/slides/section*-01-divider-*.html'):
    with open(div_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    content = fix_topic_list(content)
    
    if 'section4-01-divider-ui.html' in div_file:
        content = content.replace('<div class="part-num">PART 05</div>', '<div class="part-num">PART 04</div>')
        content = content.replace('<div class="huge-number">05</div>', '<div class="huge-number">04</div>')
        
    if 'section5-01-divider-algorithm.html' in div_file:
        content = content.replace('<div class="part-num">PART 04</div>', '<div class="part-num">PART 05</div>')
        content = content.replace('<div class="huge-number">04</div>', '<div class="huge-number">05</div>')

    with open(div_file, 'w', encoding='utf-8') as f:
        f.write(content)

agenda_path = glob.glob('*文档/slides/section0-02-agenda.html')[0]
with open(agenda_path, 'r', encoding='utf-8') as f:
    content = f.read()

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

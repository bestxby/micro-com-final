# -*- coding: utf-8 -*-
import glob

for filepath in glob.glob('演示文档/slides/section*-01-divider*.html'):
    with open(filepath, 'r', encoding='utf-8') as f:
        html = f.read()
    
    if '与<br>' in html:
        html = html.replace('与<br>', '<br>与')
        with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
            f.write(html)
        print(f"SUCCESS: Updated {filepath}")

print("All done!")

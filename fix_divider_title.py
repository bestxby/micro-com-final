# -*- coding: utf-8 -*-
import glob

# 1. Update HTML files for divider pages
for filepath in glob.glob('演示文档/slides/section*-01-divider*.html'):
    with open(filepath, 'r', encoding='utf-8') as f:
        html = f.read()
    
    # 1. Update the CSS for .part-title
    old_css = '''  .part-title {
    font-family: var(--font-title-family);
    font-size: 44px;
    font-weight: 900;
    color: var(--text-primary);
    line-height: 1.3;
  }'''
    
    new_css = '''  .part-title {
    font-family: var(--font-title-family);
    font-size: 44px;
    font-weight: 900;
    background: var(--brand-gradient);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
    line-height: 1.3;
  }'''
    
    html = html.replace(old_css, new_css)
    
    # 2. Remove <span class="highlight-gradient"> from the part-title div
    lines = html.split('\n')
    modified = False
    for i in range(len(lines)):
        if 'class="part-title"' in lines[i]:
            if '<span class="highlight-gradient">' in lines[i]:
                lines[i] = lines[i].replace('<span class="highlight-gradient">', '')
                modified = True
            if '</span>' in lines[i]:
                lines[i] = lines[i].replace('</span>', '')
                modified = True
    
    with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines))
    print(f"SUCCESS: Updated {filepath}")

print("All done!")

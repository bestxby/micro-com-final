# -*- coding: utf-8 -*-
import glob
import re

# 1. Update tokens.css
tokens_path = '演示文档/shared/tokens.css'
with open(tokens_path, 'r', encoding='utf-8') as f:
    css = f.read()

old_h1 = '''h1.slide-title {
  font-family: var(--font-title-family);
  font-size: 52px; /* Slightly reduced from 56px */
  font-weight: 900;
  line-height: 1.2;
  letter-spacing: -0.01em;
  margin-bottom: 6px;
  color: var(--text-primary);
}'''

new_h1 = '''h1.slide-title {
  font-family: var(--font-title-family);
  font-size: 52px; /* Slightly reduced from 56px */
  font-weight: 900;
  line-height: 1.2;
  letter-spacing: -0.01em;
  margin-bottom: 6px;
  background: var(--brand-gradient);
  -webkit-background-clip: text;
  background-clip: text;
  -webkit-text-fill-color: transparent;
}'''

if old_h1 in css:
    css = css.replace(old_h1, new_h1)
    with open(tokens_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(css)
    print("SUCCESS: Updated tokens.css")
else:
    print("WARNING: Could not find old h1 definition in tokens.css")

# 2. Update HTML files
for filepath in glob.glob('演示文档/slides/*.html'):
    with open(filepath, 'r', encoding='utf-8') as f:
        html = f.read()
    
    lines = html.split('\n')
    modified = False
    for i in range(len(lines)):
        if '<h1 class="slide-title">' in lines[i]:
            if '<span class="highlight-gradient">' in lines[i]:
                lines[i] = lines[i].replace('<span class="highlight-gradient">', '')
                modified = True
            if '</span>' in lines[i]:
                # Only remove </span> if it was matched, or just globally from this line
                lines[i] = lines[i].replace('</span>', '')
                modified = True
    
    if modified:
        with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
            f.write('\n'.join(lines))
        print(f"SUCCESS: Updated {filepath}")

print("All done!")

# -*- coding: utf-8 -*-
import os, glob

# Find base dir
base_dir = [d for d in glob.glob('*文档') if '展示' in d][0]

# All HTML files
html_files = glob.glob(os.path.join(base_dir, 'slides', '*.html'))
html_files.append(os.path.join(base_dir, 'index.html'))

for f in html_files:
    if os.path.exists(f):
        with open(f, 'r', encoding='utf-8') as file:
            content = file.read()
        
        # Replace tokens.css?v=4 with tokens.css?v=5
        # And base.css?v=X just in case, but let's just do v=4 -> v=5 for tokens
        if 'tokens.css?v=4' in content:
            content = content.replace('tokens.css?v=4', 'tokens.css?v=5')
            with open(f, 'w', encoding='utf-8') as file:
                file.write(content)

print("SUCCESS: Cache buster updated to v=5")

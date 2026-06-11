# -*- coding: utf-8 -*-
import glob, re

# Update tokens.css
tokens_path = glob.glob('*文档/shared/tokens.css')[0]
with open(tokens_path, 'r', encoding='utf-8') as f:
    css = f.read()

# Replace bottom: 65px with bottom: 85px
css = css.replace('bottom: 65px;', 'bottom: 85px;')

with open(tokens_path, 'w', encoding='utf-8') as f:
    f.write(css)

# Bump cache to v=7
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        c = file.read()
    c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=7', c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(c)

print("SUCCESS: Button moved to 85px and cache bumped to v=7")

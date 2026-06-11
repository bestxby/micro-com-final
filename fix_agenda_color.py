# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/slides/section0-02-agenda.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Replace the gradient text css with solid cyan color
old_css = """    background: var(--brand-gradient);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;"""

new_css = "    color: var(--brand-secondary);"

if old_css in c:
    c = c.replace(old_css, new_css)
else:
    # try replacing line by line just in case
    c = c.replace('background: var(--brand-gradient);', 'color: var(--brand-secondary);')
    c = c.replace('-webkit-background-clip: text;', '')
    c = c.replace('background-clip: text;', '')
    c = c.replace('-webkit-text-fill-color: transparent;', '')

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

# Bump cache to v=14
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        file_c = file.read()
    file_c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=14', file_c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(file_c)

print("SUCCESS: Fixed agenda number colors")

# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section3-02-bus-overview.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Increase th font size
c = c.replace('font-size: 20px;\n    font-weight: 800;', 'font-size: 24px;\n    font-weight: 800;')

# Increase td font size
c = c.replace('font-size: 20px;\n    color: var(--text-secondary);', 'font-size: 23px;\n    color: var(--text-secondary);')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Increased font size on page 9")

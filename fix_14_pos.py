# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section4-04-hmi-controls.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

c = c.replace('padding: 40px 60px;', 'padding: 0px 60px 40px; margin-top: -15px;')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Moved cards up on page 14")

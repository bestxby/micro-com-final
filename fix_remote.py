# -*- coding: utf-8 -*-
import os, glob

controls_path = glob.glob('*文档/slides/section5-04-hmi-controls.html')[0]
with open(controls_path, 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace('height: 154px;', 'height: 176px;')
content = content.replace('gap: 6px 4px; margin-bottom: 12px;', 'gap: 5px 4px; margin-bottom: 10px;')
# Also ensure the scale matches well
# We might want to remove scale or change to 0.7 if it gets too tall
content = content.replace('transform: scale(0.65);', 'transform: scale(0.70);')

with open(controls_path, 'w', encoding='utf-8') as f:
    f.write(content)
print('SUCCESS')

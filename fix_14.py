# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section4-04-hmi-controls.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Increase <p> font size
c = c.replace('font-size: 18px;', 'font-size: 22px;')

# Increase <h2> font size
c = c.replace('font-size: 28px;', 'font-size: 32px;')

# Increase <button> font size
c = c.replace('font-size: 16px;', 'font-size: 20px;')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Increased font sizes on page 14")

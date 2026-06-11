# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section4-02-hud-pages-overview.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Remove overflow: hidden; and padding-top: 10px; Add margin-top: -15px to shift up
c = c.replace('overflow: hidden; padding-top: 10px;', 'margin-top: -15px;')

# 2. Adjust grid scaling and gaps
c = c.replace('transform: scale(0.68);', 'transform: scale(0.85);')
c = c.replace('width: 1950px;', 'width: 1860px;')
c = c.replace('gap: 60px 40px;', 'gap: 20px 20px;')

# 3. Adjust gap between LCD and its label
c = c.replace("wrapper.style.gap = '24px';", "wrapper.style.gap = '12px';")

# 4. Adjust label font size slightly to fit the new tighter gap
c = c.replace("label.style.fontSize = '38px';", "label.style.fontSize = '34px';")

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Maximized layout scale on page 12")

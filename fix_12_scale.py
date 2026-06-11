# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section4-02-hud-pages-overview.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Scale down slightly and move down slightly
c = c.replace('top: 0; transform: translateX(-50%) scale(0.72);', 'top: 20px; transform: translateX(-50%) scale(0.68);')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Scaled down grid on page 12")

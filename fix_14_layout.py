# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section4-04-hmi-controls.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Container padding/margin
c = c.replace('padding: 0px 60px 40px; margin-top: -15px;', 'padding: 10px 60px 20px;')

# 2. Card padding
c = c.replace('padding: 40px; text-align: center;', 'padding: 25px 30px; text-align: center;')

# 3. h2 margin-bottom
c = c.replace('margin-bottom: 24px; font-size: 32px;', 'margin-bottom: 12px; font-size: 32px;')

# 4. <p> padding
c = c.replace('padding: 20px; border-radius: 12px;', 'padding: 16px; border-radius: 12px;')

# 5. Emoji container heights (1 and 2)
c = c.replace('<div style="height: 160px; display: flex; align-items: center; justify-content: center; font-size: 80px;">', '<div style="height: 120px; display: flex; align-items: center; justify-content: center; font-size: 80px;">')

# 6. Remote container height
c = c.replace('<div style="height: 160px; display: flex; align-items: center; justify-content: center;">', '<div style="height: 140px; display: flex; align-items: center; justify-content: center; transform: scale(0.85); transform-origin: center;">')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Fixed layout overflow on page 14")

# -*- coding: utf-8 -*-
import glob
import re

path = glob.glob('演示文档/slides/section2-03-pin-reconfig.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Increase font size of desc from 19px to 21px
c = c.replace('font-size: 19px;', 'font-size: 21px;')

# 2. Reduce padding of conflict-card to save vertical space
c = c.replace('padding: 24px 28px;', 'padding: 18px 24px;')

# 3. Reduce gap in left-list
c = c.replace('gap: 24px;', 'gap: 16px;')

# 4. Decrease h2 margin-bottom
c = c.replace('margin-bottom: 24px; border-left: 5px', 'margin-bottom: 12px; border-left: 5px')

# 5. Fix alignment of conflict-title (allow wrap if needed)
c = c.replace('justify-content: space-between;\n    align-items: center;', 'justify-content: space-between;\n    align-items: flex-start;\n    gap: 10px;')
c = c.replace('font-size: 24px;', 'font-size: 26px; line-height: 1.4;')

# 6. Shrink MCU package slightly so it doesn't overflow
c = c.replace('width: 420px; /* Increased from 320px */', 'width: 380px;')
c = c.replace('height: 420px; /* Increased from 320px */', 'height: 380px;')

# 7. Adjust pin left/right values for the smaller MCU
# Top/Bottom pins
c = c.replace('left: 80px;', 'left: 60px;')
c = c.replace('left: 230px;', 'left: 210px;')
# Left/Right pins
c = c.replace('top: 100px;', 'top: 80px;')
c = c.replace('top: 280px;', 'top: 240px;')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Adjusted layout on page 7")

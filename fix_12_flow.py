# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section4-02-hud-pages-overview.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Update the wrapper to position relative
old_wrapper = '<div style="width: 100%; height: 100%; display: flex; justify-content: center; align-items: flex-start; margin-top: -15px;">'
new_wrapper = '<div style="position: relative; width: 100%; height: 100%;">'
c = c.replace(old_wrapper, new_wrapper)

# 2. Update the grid to position absolute so it doesn't push the footer down
old_grid = '<div style="transform: scale(0.85); transform-origin: center top; width: 1860px; display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px 20px; justify-items: center;" id="grid-container">'
new_grid = '<div style="position: absolute; left: 50%; top: 0; transform: translateX(-50%) scale(0.72); transform-origin: top center; width: 1950px; display: grid; grid-template-columns: repeat(3, 1fr); gap: 50px 40px; justify-items: center;" id="grid-container">'
c = c.replace(old_grid, new_grid)

# 3. Restore the gap and font size
c = c.replace("wrapper.style.gap = '12px';", "wrapper.style.gap = '24px';")
c = c.replace("label.style.fontSize = '34px';", "label.style.fontSize = '38px';")

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Fixed layout flow on page 12")

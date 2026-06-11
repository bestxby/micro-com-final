# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/index.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Update CSS
c = re.sub(r'#stage\s*\{[^}]+\}', '''#stage {
    position: fixed;
    background: #FCFAF7;
    box-shadow: 0 25px 50px -12px rgba(25, 24, 22, 0.15);
    border-radius: 12px;
    overflow: hidden;
  }''', c)

# 2. Update fit() function
old_fit = '''  function fit() {
    currentScale = Math.min(window.innerWidth / W, window.innerHeight / H);
    currentX = (window.innerWidth  - W * currentScale) / 2;
    currentY = (window.innerHeight - H * currentScale) / 2;
    stage.style.transform = `translate(${currentX}px, ${currentY}px) scale(${currentScale})`;
    stage.style.top = '0';
    stage.style.left = '0';
  }'''

new_fit = '''  function fit() {
    currentScale = Math.min(window.innerWidth / W, window.innerHeight / H);
    // 使用 zoom 替代 transform scale，实现真矢量高像素渲染
    stage.style.zoom = currentScale;
    
    const visualW = W * currentScale;
    const visualH = H * currentScale;
    const leftPx = (window.innerWidth - visualW) / 2;
    const topPx = (window.innerHeight - visualH) / 2;
    
    // Chrome中，left和top的值也会被zoom等比例缩放，所以要除以 currentScale 以抵消影响
    stage.style.left = (leftPx / currentScale) + 'px';
    stage.style.top = (topPx / currentScale) + 'px';
    stage.style.transform = 'none';
  }'''

# 3. Update show() function which might have `stage.style.transform`
old_show_transform = 'stage.style.transform = `translate(${currentX}px, ${currentY}px) scale(${currentScale})`;'
new_show_transform = 'stage.style.transform = \'none\';'

if old_fit in c:
    c = c.replace(old_fit, new_fit)
else:
    print("WARNING: fit() not exactly matched. Regex fallback...")
    c = re.sub(r'function fit\(\)\s*\{[^\}]+\}', new_fit, c)

c = c.replace(old_show_transform, new_show_transform)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Updated index.html for native crisp resolution")

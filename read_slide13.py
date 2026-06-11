import glob
import re

path = glob.glob('*文档/slides/section4-03-hud-ui-design.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

m = re.search(r'<div class="left-intro">(.*?)<div class="right-screens">', content, re.DOTALL)
if m:
    print(m.group(1))

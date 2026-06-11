# -*- coding: utf-8 -*-
import os, glob

slides_dir = glob.glob('*文档/slides')[0]

# Update Slide 13 (section4-03-hud-ui-design.html)
slide13_path = os.path.join(slides_dir, 'section4-03-hud-ui-design.html')
with open(slide13_path, 'r', encoding='utf-8') as f:
    content13 = f.read()

# I previously injected:
# .left-col p { font-size: 21px; line-height: 1.8; }
# .left-col li { font-size: 22px; line-height: 1.8; }
# .left-col strong { font-size: 25px; }
content13 = content13.replace('font-size: 21px;', 'font-size: 23px;')
content13 = content13.replace('font-size: 22px;', 'font-size: 23px;')
content13 = content13.replace('font-size: 25px;', 'font-size: 26px;')

with open(slide13_path, 'w', encoding='utf-8') as f:
    f.write(content13)

# Update Slide 17 (section5-03-dirty-rects.html)
slide17_path = os.path.join(slides_dir, 'section5-03-dirty-rects.html')
with open(slide17_path, 'r', encoding='utf-8') as f:
    content17 = f.read()

# Slide 17 probably uses .left-col or .algo-list. Let's just inject a rule at the end of <style>
# that forces paragraphs and list items to be 23px, and strong to be 26px.
style_inject = """
  p, li { font-size: 23px !important; line-height: 1.8; }
  strong { font-size: 26px !important; }
  .code-badge { font-size: 20px !important; } /* In case it has code badges */
"""

if 'font-size: 23px !important;' not in content17:
    content17 = content17.replace('</style>', style_inject + '\n</style>')

with open(slide17_path, 'w', encoding='utf-8') as f:
    f.write(content17)

print("SUCCESS")

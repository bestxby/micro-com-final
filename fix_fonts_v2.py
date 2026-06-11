# -*- coding: utf-8 -*-
import os, glob

slides_dir = glob.glob('*文档/slides')[0]

def enforce_left_intro_font(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # The robust CSS that will override the list item sizes
    correct_css = """
  /* FORCE ALIGN TEXT SIZE */
  .left-intro p, .left-intro li, .design-points li { font-size: 23px !important; line-height: 1.8; }
  .left-intro strong, .design-points strong { font-size: 26px !important; }
"""
    if '/* FORCE ALIGN TEXT SIZE */' not in content:
        content = content.replace('</style>', correct_css + '\n</style>')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

enforce_left_intro_font(os.path.join(slides_dir, 'section4-03-hud-ui-design.html'))
enforce_left_intro_font(os.path.join(slides_dir, 'section5-03-dirty-rects.html'))

print("SUCCESS")

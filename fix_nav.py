# -*- coding: utf-8 -*-
import os, glob, re

# 1. Update index.html navigation buttons
index_path = glob.glob('*文档/index.html')[0]
with open(index_path, 'r', encoding='utf-8') as f:
    idx_content = f.read()

# Replace the nav-zone css block
# Find start of .nav-zone
start = idx_content.find('.nav-zone {')
# Find end of hover block
end = idx_content.find('}', idx_content.find('.nav-zone:hover .nav-hint {')) + 1

if start != -1 and end != -1:
    new_nav_css = """  .nav-zone {
    position: fixed;
    top: 50%; transform: translateY(-50%);
    width: 60px; height: 60px;
    cursor: pointer;
    z-index: 50;
  }
  .nav-zone.left  { left: 30px; }
  .nav-zone.right { right: 30px; }
  .nav-hint {
    position: absolute;
    top: 0; left: 0; width: 100%; height: 100%;
    border-radius: 999px;
    background: rgba(25, 24, 22, 0.08);
    border: 1px solid rgba(25, 24, 22, 0.15);
    color: rgba(25, 24, 22, 0.6);
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 28px;
    font-weight: 300;
    opacity: 0.4;
    transition: all 0.2s ease;
  }
  .nav-zone:hover .nav-hint {
    opacity: 1;
    background: rgba(25, 24, 22, 0.15);
    color: var(--text-primary);
  }"""
    idx_content = idx_content[:start] + new_nav_css + idx_content[end:]
    
    with open(index_path, 'w', encoding='utf-8') as f:
        f.write(idx_content)


# 2. Update the video button to move it back to the bottom right
overview_path = glob.glob('*文档/slides/section4-02-hud-pages-overview.html')[0]
with open(overview_path, 'r', encoding='utf-8') as f:
    ov_content = f.read()

# It's currently at top: 35px; right: 320px;
# Change it to bottom: 80px; right: 50px;
ov_content = ov_content.replace('top: 35px; right: 320px;', 'bottom: 80px; right: 50px;')
# Also adjust the hover translateY back if it was upwards
# The button already has translation, let's keep it.

with open(overview_path, 'w', encoding='utf-8') as f:
    f.write(ov_content)

print("SUCCESS")

# -*- coding: utf-8 -*-
import os, glob, re

path = glob.glob('*文档/slides/section4-04-hmi-controls.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

# The button HTML was:
# <button class="view-video-btn">
#   <svg ...
#   查看视频演示
# </button>
# It is located right before <div class="slide-footer">

start = content.find('<button class="view-video-btn">')
if start != -1:
    end = content.find('</button>', start) + 9
    content = content[:start] + content[end:]
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print("SUCCESS")
else:
    print("BUTTON NOT FOUND")

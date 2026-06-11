# -*- coding: utf-8 -*-
import glob
import re

path = glob.glob('演示文档/slides/section1-02-background.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Remove the entire "痛点背景" <li>
old_li1 = '<li><strong>痛点背景</strong>：传统居家安防监测设备存在误报率高、抗噪性弱、网络掉线挂死、无法本地与云端数据协同等设计瓶颈。</li>'
c = c.replace(old_li1 + '\n', '')
c = c.replace(old_li1, '')

# 2. Remove all <strong> and </strong> tags from the remaining content
c = c.replace('<strong>', '')
c = c.replace('</strong>', '')

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Deleted background info and removed bold tags on page 4")

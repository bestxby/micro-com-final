# -*- coding: utf-8 -*-
import os, glob

path = glob.glob('*文档/slides/section4-03-hud-ui-design.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

# Remove "(Card-UI)"
content = content.replace('模块化卡片架构 (Card-UI)', '模块化卡片架构')

# Remove the checkmarks
# The CSS has:
#   .design-points li::before {
#     content: "✓";
#     position: absolute;
#     left: 0;
#     color: var(--brand-secondary);
#     font-weight: 900;
#   }
# I will just change content: "✓"; to content: "";
content = content.replace('content: "✓";', 'content: "";')
content = content.replace('padding-left: 28px;', 'padding-left: 0;')

with open(path, 'w', encoding='utf-8') as f:
    f.write(content)

print("SUCCESS")

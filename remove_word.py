# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/slides/section4-03-hud-ui-design.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

# Replace '关键数据' with '数据'
content = content.replace('等关键数据进行区块化精准隔离', '等数据进行区块化精准隔离')

with open(path, 'w', encoding='utf-8') as f:
    f.write(content)

print("SUCCESS")

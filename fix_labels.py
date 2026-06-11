# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/index.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Fix the broken labels in DECK_MANIFEST
c = c.replace('[页] 04.', '[引导] 04.')
c = c.replace('[分] 05.', '[引导] 05.')

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("SUCCESS: Fixed broken labels in DECK_MANIFEST")

# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/slides/section0-02-agenda.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Replace fonts
c = c.replace('font-size: 36px;', 'font-size: 46px;')
c = c.replace('font-size: 44px;', 'font-size: 54px;') # The 7th span 3 element num
c = c.replace('font-size: 20px;', 'font-size: 26px;')
c = c.replace('font-size: 15px;', 'font-size: 20px;')

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

# Bump cache to v=13
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        file_c = file.read()
    file_c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=13', file_c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(file_c)

print("SUCCESS: Increased fonts on agenda slide")

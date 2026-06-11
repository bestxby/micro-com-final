# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/slides/section7-02-joint-verification.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Increase metric table font size
c = c.replace('font-size: 20px;', 'font-size: 23px;')

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

# Bump cache to v=16
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        file_c = file.read()
    file_c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=16', file_c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(file_c)

print("SUCCESS: Fixed table font size on Slide 22")

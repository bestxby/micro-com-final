# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/index.html')[0]

with open(path, 'rb') as f:
    c = f.read()

# Replace all \n\n with \n repeatedly until clean
while b'\n\n' in c:
    c = c.replace(b'\n\n', b'\n')

with open(path, 'wb') as f:
    f.write(c)

print("SUCCESS: Cleaned up newlines")

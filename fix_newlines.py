# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/index.html')[0]

# Read binary, remove all \r
with open(path, 'rb') as f:
    c = f.read()

c = c.replace(b'\r', b'')

with open(path, 'wb') as f:
    f.write(c)

print("SUCCESS: Fixed newlines in index.html")

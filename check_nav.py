import glob
with open(glob.glob('*文档/index.html')[0], 'r', encoding='utf-8') as f:
    content = f.read()
print(content[-1500:])

import glob, re

for f in glob.glob('*文档/slides/section[45]*.html'):
    with open(f, 'r', encoding='utf-8') as file:
        content = file.read()
    m = re.search(r'<div class="section-name">(.*?)</div>', content)
    name = m.group(1) if m else "NONE"
    print(f"{f}: {name}")

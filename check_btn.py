import glob, re

path = glob.glob('*文档/slides/section6-02-home-security.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

m = re.search(r'<button class="view-video-btn">.*?</button>', content, re.DOTALL)
if m:
    print(m.group(0))
else:
    print("NOT FOUND")

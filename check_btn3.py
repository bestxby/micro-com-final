import glob, re

path = glob.glob('*文档/slides/section6-03-wechat-logging.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

m = re.search(r'<button class="view-video-btn".*?</button>', content, re.DOTALL)
if m:
    with open('found_btn.txt', 'w', encoding='utf-8') as out:
        out.write(m.group(0))
    print("SAVED TO found_btn.txt")
else:
    print("NOT FOUND")

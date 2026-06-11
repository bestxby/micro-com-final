# -*- coding: utf-8 -*-
import glob, re

# 1. Update tokens.css
tokens_path = glob.glob('*文档/shared/tokens.css')[0]
with open(tokens_path, 'r', encoding='utf-8') as f:
    css = f.read()

# Replace bottom: 65px with bottom: 85px
css = css.replace('bottom: 65px;', 'bottom: 85px;')

with open(tokens_path, 'w', encoding='utf-8') as f:
    f.write(css)

# 2. Delete button from Slide 22
slide22_path = glob.glob('*文档/slides/section7-02-joint-verification.html')[0]
with open(slide22_path, 'r', encoding='utf-8') as f:
    c22 = f.read()

# Try to find and remove the button
if '<button class="view-video-btn"' in c22:
    start_btn = c22.find('<button class="view-video-btn"')
    end_btn = c22.find('</button>', start_btn) + 9
    c22 = c22[:start_btn] + c22[end_btn:]
    print("Deleted view-video-btn from Slide 22")
elif '播放综合演示视频' in c22:
    # Maybe it's the huge old button?
    start_btn = c22.find('<button')
    end_btn = c22.find('</button>', start_btn) + 9
    c22 = c22[:start_btn] + c22[end_btn:]
    print("Deleted generic button from Slide 22")

with open(slide22_path, 'w', encoding='utf-8') as f:
    f.write(c22)

# Bump cache to v=7
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        c = file.read()
    c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=7', c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(c)

print("SUCCESS")

# -*- coding: utf-8 -*-
import glob, re

for f in glob.glob('演示文档/slides/section*-divider*.html'):
    c = open(f, encoding='utf-8').read()
    m = re.search(r'<div class="topic-item active-topic">\s*<div class="topic-num">(.*?)</div>', c)
    if m:
        print(f, m.group(1))
    else:
        print(f, "NOT FOUND")

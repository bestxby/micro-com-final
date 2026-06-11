# -*- coding: utf-8 -*-
import glob
import json

results = {}
for f in glob.glob('演示文档/slides/section*-divider*.html'):
    lines = open(f, encoding='utf-8').read().split('\n')
    for line in lines:
        if '<div class="part-title">' in line:
            results[f] = line.strip()

with open('titles.json', 'w', encoding='utf-8') as out:
    json.dump(results, out, ensure_ascii=False, indent=2)

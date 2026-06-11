# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section6-02-home-security.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

old_str = '并立即触发 <strong>微信警报推送 (180s 冷却防抖限制)</strong>。'
new_str = '并立即触发微信警报推送。'

if old_str in c:
    c = c.replace(old_str, new_str)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(c)
    print("SUCCESS: Replaced string")
else:
    print("WARNING: String not found. Let's try finding a substring.")
    # Fallback if there are spaces or linebreaks
    import re
    c = re.sub(r'并立即触发\s*<strong>\s*微信警报推送\s*\(180s\s*冷却防抖限制\)\s*</strong>。', '并立即触发微信警报推送。', c)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(c)
    print("Fallback regex run.")

# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section6-03-wechat-logging.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

old_str = '状态机软锁（<strong>180s 物理冷却期</strong>），静默拦截'
new_str = '状态机软锁（180s 物理冷却期），静默拦截'

if old_str in c:
    c = c.replace(old_str, new_str)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(c)
    print("SUCCESS: Removed bold from 180s in page 20")
else:
    print("WARNING: String not found.")

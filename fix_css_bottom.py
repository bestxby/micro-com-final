# -*- coding: utf-8 -*-
import glob

tokens_path = glob.glob('*文档/shared/tokens.css')[0]
with open(tokens_path, 'r', encoding='utf-8') as f:
    css = f.read()

start = css.find('.view-video-btn {')
end = css.find('}', start)
btn_block = css[start:end]

# It has "bottom: 110px;"
if 'bottom: 110px;' in btn_block:
    new_btn_block = btn_block.replace('bottom: 110px;', 'bottom: 65px;')
    css = css[:start] + new_btn_block + css[end:]
    with open(tokens_path, 'w', encoding='utf-8') as f:
        f.write(css)
    print("SUCCESS: Changed bottom to 65px")
else:
    print("bottom: 110px not found in view-video-btn!")

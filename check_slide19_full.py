import glob

with open(glob.glob('*文档/slides/section6-02-home-security.html')[0], 'r', encoding='utf-8') as f:
    c = f.read()

has_modal = 'id="videoModal"' in c
has_script = '<script>' in c
has_inline_btn_css = '.view-video-btn {' in c

print("Has Modal:", has_modal)
print("Has Script:", has_script)
print("Has Inline Button CSS:", has_inline_btn_css)

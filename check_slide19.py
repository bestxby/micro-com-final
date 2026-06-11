import glob
with open(glob.glob('*文档/slides/section6-02-home-security.html')[0], 'r', encoding='utf-8') as f:
    c = f.read()

b = c.find('<button class="view-video-btn"')
if b != -1:
    print(c[b:c.find('</div>', b)+400])

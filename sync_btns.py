import glob

path = glob.glob('*文档/slides/section6-02-home-security.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

start = c.find('<button class="view-video-btn"')
end = c.find('</button>', start) + 9

btn_html = c[start:end]

targets = [
    'section4-02-hud-pages-overview.html',
    'section4-03-hud-ui-design.html',
    'section5-02-edge-algorithms.html',
    'section5-03-dirty-rects.html'
]

for t in targets:
    p = glob.glob(f'*文档/slides/{t}')[0]
    with open(p, 'r', encoding='utf-8') as f:
        tc = f.read()
    
    # If the button exists, replace it
    if '<button class="view-video-btn"' in tc:
        s = tc.find('<button class="view-video-btn"')
        e = tc.find('</button>', s) + 9
        tc = tc[:s] + btn_html + tc[e:]
    else:
        # Inject before slide-footer
        tc = tc.replace('<div class="slide-footer">', btn_html + '\n<div class="slide-footer">')
        
    with open(p, 'w', encoding='utf-8') as f:
        f.write(tc)

print("SUCCESS: Synced verbatim to 12, 13, 16, 17")

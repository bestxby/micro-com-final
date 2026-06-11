import glob
with open(glob.glob('*文档/shared/tokens.css')[0], 'r', encoding='utf-8') as f:
    css = f.read()
start = css.find('.view-video-btn')
print("tokens.css snippet:")
print(css[start:css.find('}', start)+1])

with open(glob.glob('*文档/slides/section4-03-hud-ui-design.html')[0], 'r', encoding='utf-8') as f:
    c = f.read()
s = c.find('<ul class="design-points">')
print("\nSlide 13 snippet:")
print(c[s:c.find('</ul>', s)+5])

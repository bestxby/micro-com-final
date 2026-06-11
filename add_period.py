import glob

for f in glob.glob('*文档/slides/section4-03-hud-ui-design.html'):
    with open(f, 'r', encoding='utf-8') as file:
        content = file.read()
    
    # ensure it has a period
    content = content.replace("打造现代智能终端", "打造现代智能终端。")
    # avoid double period if it somehow got added twice
    content = content.replace("打造现代智能终端。。", "打造现代智能终端。")
    
    with open(f, 'w', encoding='utf-8') as file:
        file.write(content)
    print("SUCCESS")

import glob

target_text = "打造媲美现代智能终端的流畅体验"
replacement = "打造现代智能终端"

found = False
for f in glob.glob('*文档/slides/*.html'):
    with open(f, 'r', encoding='utf-8') as file:
        content = file.read()
    
    if target_text in content:
        content = content.replace(target_text, replacement)
        # Also replace if it includes the punctuation
        content = content.replace("打造现代智能终端。", "打造现代智能终端") # Just in case
        with open(f, 'w', encoding='utf-8') as file:
            file.write(content)
        print(f"Replaced in {f}")
        found = True

if not found:
    print("Text not found in any slide.")

import glob
with open(glob.glob('*文档/slides/section6-02-home-security.html')[0], 'r', encoding='utf-8') as f:
    c = f.read()

start_style = c.find('<style>')
end_style = c.find('</style>')
css = c[start_style:end_style]

start_modal = c.find('<div class="video-modal"')
if start_modal == -1:
    start_modal = c.find('<div id="videoModal"')
end_modal = c.find('</div>', c.rfind('</div>', 0, c.find('<script>'))) # heuristics
# actually let's just grab from videoModal to <script>
script_start = c.find('<script>')
end_modal = script_start

modal_html = c[start_modal:end_modal]
script_html = c[script_start:c.find('</script>')+9]

with open('extracted_19.txt', 'w', encoding='utf-8') as f:
    f.write("=== CSS ===\n" + css + "\n=== MODAL ===\n" + modal_html + "\n=== SCRIPT ===\n" + script_html)

print("SUCCESS")

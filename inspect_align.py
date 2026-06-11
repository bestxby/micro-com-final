import glob, re

for f in glob.glob('*文档/slides/section[45]*.html') + glob.glob('*文档/slides/section0-02-agenda.html') + glob.glob('*文档/index.html'):
    print(f"--- {f} ---")
    with open(f, 'r', encoding='utf-8') as file:
        content = file.read()
    
    # 1. <title>
    title_match = re.search(r'<title>(.*?)</title>', content)
    if title_match: print("  Title:", title_match.group(1))
    
    # 2. .part-num
    part_match = re.search(r'<div class="part-num">(.*?)</div>', content)
    if part_match: print("  Part Num:", part_match.group(1))
    
    # 3. .huge-number
    huge_match = re.search(r'<div class="huge-number">(.*?)</div>', content)
    if huge_match: print("  Huge Num:", huge_match.group(1))
    
    # 4. .section-name
    sec_match = re.search(r'<div class="section-name">(.*?)</div>', content)
    if sec_match: print("  Section Name:", sec_match.group(1))
    
    # 5. active-topic text
    active_match = re.search(r'<div class="topic-item active-topic">.*?<div class="topic-text">(.*?)</div>', content, re.DOTALL)
    if active_match: print("  Active Topic:", active_match.group(1).strip())

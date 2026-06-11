import glob
with open(glob.glob('*文档/slides/section6-02-home-security.html')[0], 'r', encoding='utf-8') as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    if 'view-video-btn' in line:
        print(f"Line {i}:")
        for j in range(max(0, i-5), min(len(lines), i+5)):
            print(f"{j}: {lines[j].strip()}")
        break

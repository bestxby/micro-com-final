# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/slides/section6-02-home-security.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Replace "Safe &harr; Warning &harr; Alarm" with colored versions
old_str = "<strong>Safe &harr; Warning &harr; Alarm</strong>"
new_str = "<strong><span style=\"color: var(--brand-secondary);\">Safe</span> &harr; <span style=\"color: var(--brand-primary);\">Warning</span> &harr; <span style=\"color: var(--alert-red);\">Alarm</span></strong>"
c = c.replace(old_str, new_str)

# Replace the single "Safe" at the end
old_safe = "重置为 Safe 状态"
new_safe = "重置为 <strong style=\"color: var(--brand-secondary);\">Safe</strong> 状态"
c = c.replace(old_safe, new_safe)

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

# Bump cache to v=17
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        file_c = file.read()
    file_c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=17', file_c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(file_c)

print("SUCCESS: Colored Safe/Warning/Alarm")

# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/slides/section0-02-agenda.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# First, remove the previously injected script
c = re.sub(r'<script>.*?querySelectorAll\(\'.agenda-card\'\).*?</script>', '', c, flags=re.DOTALL)

# Remove the inline onclick from the div tags
c = re.sub(r' onclick="if\(window\.parent\) window\.parent\.postMessage\(\{goto: \d+\}, \'\*\'\)"', '', c)

# Replace the glass-card structure
targets = [3, 5, 8, 11, 15, 18, 21]

for i, num in enumerate(['01', '02', '03', '04', '05', '06', '07']):
    target_hash = f"../index.html#{targets[i]}"
    
    # We find the specific card
    if num == '07':
        pattern = rf'(<div class="glass-card agenda-card">)\s*(<div class="agenda-num">07</div>)\s*(<div>)\s*(<div class="agenda-title">.*?</div>)\s*(<div class="agenda-desc">.*?</div>)\s*(</div>)\s*(</div>)'
    else:
        pattern = rf'(<div class="glass-card agenda-card">)\s*(<div class="agenda-num">{num}</div>)\s*(<div class="agenda-title">.*?</div>)\s*(<div class="agenda-desc">.*?</div>)\s*(</div>)'
        
    def replacer(m):
        inner = "".join(m.groups())
        return f'<a href="{target_hash}" target="_parent" style="text-decoration: none; color: inherit; display: block; outline: none; border: none; height: 100%;">{inner}</a>'
        
    c = re.sub(pattern, replacer, c, flags=re.DOTALL)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Converted all cards to absolute parent navigation links")

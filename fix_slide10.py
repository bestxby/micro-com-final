# -*- coding: utf-8 -*-
import os, glob

slide10_path = glob.glob('*文档/slides/section3-03-protocol-drivers.html')[0]
with open(slide10_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Change opt-title color to primary (green)
content = content.replace('color: var(--brand-secondary);', 'color: var(--brand-primary);')

# Change opt-icon background to primary to match the green theme
content = content.replace('background: var(--brand-gradient);', 'background: var(--brand-primary);')

# Inject a style rule so that any code tag inside opt-title is also green instead of black
style_injection = """  .opt-title code.proto-tag, .opt-title code.pin-tag {
    color: var(--brand-primary) !important;
  }
</style>"""
content = content.replace('</style>', style_injection)

with open(slide10_path, 'w', encoding='utf-8') as f:
    f.write(content)
print("SUCCESS")

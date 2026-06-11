# -*- coding: utf-8 -*-
import glob, re

css_snippet = """
  /* FORCE ALIGN TEXT SIZE (Like Slide 17) */
  p, li { font-size: 23px !important; line-height: 1.8; }
  strong { font-size: 26px !important; }
"""

path = glob.glob('*文档/slides/section3-03-protocol-drivers.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Inject before </style> if not already there
if 'FORCE ALIGN TEXT SIZE' not in c:
    c = c.replace('</style>', css_snippet + '\n</style>')
    with open(path, 'w', encoding='utf-8') as f:
        f.write(c)

# Bump cache to v=15
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        file_c = file.read()
    file_c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=15', file_c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(file_c)

print("SUCCESS: Aligned fonts on 10")

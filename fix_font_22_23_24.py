# -*- coding: utf-8 -*-
import glob, re

css_snippet = """
  /* FORCE ALIGN TEXT SIZE (Like Slide 17) */
  p, li { font-size: 23px !important; line-height: 1.8; }
  strong { font-size: 26px !important; }
"""

targets = [
    'section7-02-joint-verification.html',
    'section7-03-summary.html',
    'section7-04-experience.html'
]

for t in targets:
    paths = glob.glob(f'*文档/slides/{t}')
    if not paths: continue
    path = paths[0]
    with open(path, 'r', encoding='utf-8') as f:
        c = f.read()
    
    # Inject before </style> if not already there
    if 'FORCE ALIGN TEXT SIZE' not in c:
        c = c.replace('</style>', css_snippet + '\n</style>')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(c)

# Bump cache to v=10
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        c = file.read()
    c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=10', c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(c)

print("SUCCESS: Aligned fonts on 22, 23, 24")

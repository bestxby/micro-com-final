import glob, re

path = glob.glob('*文档/slides/section5-03-dirty-rects.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

missing_css = """
  .stats-box {
    margin-top: 24px;
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 16px;
  }
  .stat-card {
    background: rgba(25, 24, 22, 0.03);
    border: 1px solid var(--card-border);
    padding: 16px;
    border-radius: 8px;
    text-align: center;
  }
  .card-label {
    font-size: 18px;
    font-weight: 700;
    color: var(--text-secondary);
    margin-bottom: 6px;
    display: block;
  }
  .stat-num {
    font-size: 32px;
    font-weight: 800;
  }
  .text-red { color: var(--alert-red); }
  .text-cyan { color: var(--brand-secondary); }
"""

# Insert before </style>
if '.stats-box {' not in c:
    c = c.replace('</style>', missing_css + '\n</style>')

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

# Bump cache to v=9
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        file_c = file.read()
    file_c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=9', file_c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(file_c)

print("SUCCESS: Restored stats-box CSS")

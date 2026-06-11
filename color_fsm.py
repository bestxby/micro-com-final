# -*- coding: utf-8 -*-
import glob, re

path = glob.glob('*文档/slides/section6-02-home-security.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Replace the FSM paragraph with a colored one
old_fsm = """<p style="font-size: 21px; color: var(--text-muted); line-height: 1.8;">
          • 撤防模式 (Disarmed) &rarr; 按 KEY3 / 红外OK &rarr; 布防监测 (Armed)<br>
          • Armed 状态下，系统持续轮询测距：当超声波距离突变触发特定边界时，直接进行 <strong>Safe &harr; Warning &harr; Alarm</strong> 物理映射。撤防操作将强制重置为 Safe 状态。
        </p>"""

new_fsm = """<p style="font-size: 21px; color: var(--text-muted); line-height: 1.8;">
          • <span style="color: var(--text-secondary); font-weight: bold;">撤防模式 (Disarmed)</span> &rarr; 按 KEY3 / 红外OK &rarr; <span style="color: var(--brand-primary); font-weight: bold;">布防监测 (Armed)</span><br>
          • Armed 状态下，系统持续轮询测距：当超声波距离突变触发特定边界时，直接进行 <span class="state-badge badge-safe" style="font-size: 18px; padding: 2px 8px;">Safe</span> &harr; <span class="state-badge badge-warning" style="font-size: 18px; padding: 2px 8px;">Warning</span> &harr; <span class="state-badge badge-alarm" style="font-size: 18px; padding: 2px 8px;">Alarm</span> 物理映射。撤防操作将强制重置为 <span class="state-badge badge-safe" style="font-size: 18px; padding: 2px 8px;">Safe</span> 状态。
        </p>"""

c = c.replace(old_fsm, new_fsm)

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

# Bump cache to v=11
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        file_c = file.read()
    file_c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=11', file_c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(file_c)

print("SUCCESS: FSM colored")

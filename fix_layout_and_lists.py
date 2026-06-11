# -*- coding: utf-8 -*-
import glob
import re

# Task 1: Fix divider widths
for filepath in glob.glob('演示文档/slides/section*-01-divider*.html'):
    with open(filepath, 'r', encoding='utf-8') as f:
        html = f.read()
    
    html = html.replace('flex: 0 0 25%;', 'flex: 0 0 32%;')
    html = html.replace('flex: 0 0 65%;', 'flex: 0 0 60%;')
    
    with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
        f.write(html)
    print(f"SUCCESS: Updated widths in {filepath}")


# Task 2: Refactor Page 20 lists
path20 = glob.glob('演示文档/slides/section6-03-wechat-logging.html')[0]
with open(path20, 'r', encoding='utf-8') as f:
    c = f.read()

# Replace col 1 desc
old_desc_1 = '''<div class="state-desc">
          通过 <code class="proto-tag">AT</code> 指令进行底层握手，断线后自动保存配置重连。<br><br>掉线期间无缝切入本地 10ms 软件 <code class="proto-tag">RTC</code> 累加，恢复网络后自动对齐秒级误差。
        </div>'''
new_desc_1 = '''<div class="state-desc">
          <ul style="list-style: none; padding: 0; margin: 0; display: flex; flex-direction: column; gap: 12px;">
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: var(--brand-secondary); font-size: 20px;">•</span>
              <span><strong>断线重连:</strong> 基于 <code class="proto-tag">AT</code> 指令底层握手，自动保存配置。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: var(--brand-secondary); font-size: 20px;">•</span>
              <span><strong>时钟自适应:</strong> 掉线无缝切入 10ms 软 <code class="proto-tag">RTC</code>，联网自动对齐。</span>
            </li>
          </ul>
        </div>'''
c = c.replace(old_desc_1, new_desc_1)

# Replace col 2 desc
old_desc_2 = '''<div class="state-desc">
          直接使用 <code>strstr</code> 从 <code class="proto-tag">TCP</code> 接收环形缓存中匹配特定键值对。<br><br>舍弃开销巨大的 <code>JSON</code> 库，极大节约了 RAM 空间。
        </div>'''
new_desc_2 = '''<div class="state-desc">
          <ul style="list-style: none; padding: 0; margin: 0; display: flex; flex-direction: column; gap: 12px;">
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #0369a1; font-size: 20px;">•</span>
              <span><strong>原生解析:</strong> <code>strstr</code> 定向匹配 <code class="proto-tag">TCP</code> 环形缓存。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #0369a1; font-size: 20px;">•</span>
              <span><strong>极致轻量:</strong> 零 <code>JSON</code> 库依赖，释放海量 RAM。</span>
            </li>
          </ul>
        </div>'''
c = c.replace(old_desc_2, new_desc_2)

# Replace col 3 desc
old_desc_3 = '''<div class="state-desc">
          遭入侵或温度突变偏差 &gt; 5℃ 时，调用 Server酱 向微信推送报警。<br><br>为防 HTTP 拥塞和频繁重入，设计了 <strong>180秒发送冷却期</strong>。
        </div>'''
new_desc_3 = '''<div class="state-desc">
          <ul style="list-style: none; padding: 0; margin: 0; display: flex; flex-direction: column; gap: 12px;">
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #047857; font-size: 20px;">•</span>
              <span><strong>触发条件:</strong> 入侵侦测 或 剧烈温变 (&gt;5℃)。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #047857; font-size: 20px;">•</span>
              <span><strong>防抖机制:</strong> <strong>180s 冷却期</strong>，杜绝 HTTP 拥塞。</span>
            </li>
          </ul>
        </div>'''
c = c.replace(old_desc_3, new_desc_3)

# Replace col 4 desc
old_desc_4 = '''<div class="state-desc">
          设计了 SD 日志扇区对齐算法。<br><br>读取或覆写时按照物理 512 字节进行对齐擦写，将含有北京时间戳的事件及环境参数实时持久化。
        </div>'''
new_desc_4 = '''<div class="state-desc">
          <ul style="list-style: none; padding: 0; margin: 0; display: flex; flex-direction: column; gap: 12px;">
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #CC5A01; font-size: 20px;">•</span>
              <span><strong>底层对齐:</strong> 严格按物理 512B 扇区擦写。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #CC5A01; font-size: 20px;">•</span>
              <span><strong>数据归档:</strong> 时间戳事件与参数实时持久化。</span>
            </li>
          </ul>
        </div>'''
c = c.replace(old_desc_4, new_desc_4)

with open(path20, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Refactored lists in Page 20")

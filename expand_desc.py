# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section6-03-wechat-logging.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Replace col 1
old_desc_1 = '''<span><strong>断线重连:</strong> 基于 <code class="proto-tag">AT</code> 指令底层握手，自动保存配置。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: var(--brand-secondary); font-size: 20px;">•</span>
              <span><strong>时钟自适应:</strong> 掉线无缝切入 10ms 软 <code class="proto-tag">RTC</code>，联网自动对齐。</span>'''

new_desc_1 = '''<span><strong>断线重连:</strong> 基于底层 <code class="proto-tag">AT</code> 指令集实现异步握手。检测到网络异常断开时，系统会自动读取配置尝试重连，确保通信链路的高可用性。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: var(--brand-secondary); font-size: 20px;">•</span>
              <span><strong>时钟自适应:</strong> 断网时系统会自动无缝切入本地 10ms 软件 <code class="proto-tag">RTC</code> 累加。一旦恢复网络连接，系统会重新拉取云端时间，并自动平滑对齐秒级误差。</span>'''
c = c.replace(old_desc_1, new_desc_1)

# Replace col 2
old_desc_2 = '''<span><strong>原生解析:</strong> <code>strstr</code> 定向匹配 <code class="proto-tag">TCP</code> 环形缓存。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #0369a1; font-size: 20px;">•</span>
              <span><strong>极致轻量:</strong> 零 <code>JSON</code> 库依赖，释放海量 RAM。</span>'''

new_desc_2 = '''<span><strong>原生解析:</strong> 针对单片机资源受限的特点，直接基于底层的 <code>strstr</code> 函数，从非阻塞 <code class="proto-tag">TCP</code> 接收的环形缓存中精准匹配特定的目标键值对。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #0369a1; font-size: 20px;">•</span>
              <span><strong>极致轻量:</strong> 彻底摒弃了开销极其庞大、易致内存碎片的第三方 <code>JSON</code> 库，这不仅将字符串处理效率提升数倍，更为微控制器释放了海量的 RAM 空间。</span>'''
c = c.replace(old_desc_2, new_desc_2)

# Replace col 3
old_desc_3 = '''<span><strong>触发条件:</strong> 入侵侦测 或 剧烈温变 (&gt;5℃)。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #047857; font-size: 20px;">•</span>
              <span><strong>防抖机制:</strong> <strong>180s 冷却期</strong>，杜绝 HTTP 拥塞。</span>'''

new_desc_3 = '''<span><strong>触发条件:</strong> 一旦超声波雷达侦测到非法入侵，或本地环境温度发生剧烈突变 (&gt;5℃)，将立即通过 Server酱 云端 <code class="proto-tag">API</code> 向用户的绑定微信推送图文警报。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #047857; font-size: 20px;">•</span>
              <span><strong>防抖机制:</strong> 为防止多次快速触发导致 HTTP 请求大量拥塞，系统在底层专门设计了一把状态机软锁（<strong>180s 物理冷却期</strong>），静默拦截一切短时间内的重复报警。</span>'''
c = c.replace(old_desc_3, new_desc_3)

# Replace col 4
old_desc_4 = '''<span><strong>底层对齐:</strong> 严格按物理 512B 扇区擦写。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #CC5A01; font-size: 20px;">•</span>
              <span><strong>数据归档:</strong> 时间戳事件与参数实时持久化。</span>'''

new_desc_4 = '''<span><strong>底层对齐:</strong> 针对 FAT32 特性，设计了基于 512B 扇区对齐的日志写入算法。无论是读取还是追加覆写，均严格按照物理边界进行整块擦写，避免破坏底层簇链。</span>
            </li>
            <li style="display: flex; align-items: flex-start; gap: 8px;">
              <span style="color: #CC5A01; font-size: 20px;">•</span>
              <span><strong>数据归档:</strong> 无论是状态机的每一次异常切换事件，还是带有精确北京时间戳的温湿度参数，都会被实时、永久地序列化保存到本地 SD 卡中。</span>'''
c = c.replace(old_desc_4, new_desc_4)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Expanded descriptions in Page 20")

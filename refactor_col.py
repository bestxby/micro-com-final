# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/slides/section6-03-wechat-logging.html')[0]
c = open(path, encoding='utf-8').read()

# We want to replace the <style> block and the <div class="layout-box"> block
old_style_start = c.find('<style>')
old_style_end = c.find('</style>') + len('</style>')

new_style = '''<style>
  .three-col-layout {
    display: flex;
    gap: 32px;
    width: 100%;
    height: 100%;
    margin-top: 10px;
    padding-bottom: 20px;
  }
  .state-card {
    flex: 1;
    background: var(--card-bg);
    border: 1px solid var(--card-border);
    box-shadow: var(--glass-shadow);
    border-radius: 20px;
    padding: 36px 32px;
    display: flex;
    flex-direction: column;
    position: relative;
    overflow: hidden;
    transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);
  }
  .state-card:hover {
    transform: translateY(-8px);
    box-shadow: 0 25px 50px -12px rgba(15, 118, 110, 0.15);
    border-color: rgba(15, 118, 110, 0.3);
  }
  .state-icon-area {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 24px;
  }
  .state-num {
    font-size: 54px;
    font-weight: 900;
    background: var(--brand-gradient);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
    font-family: var(--font-title-family);
    width: fit-content;
  }
  .state-badge {
    padding: 6px 14px;
    border-radius: 999px;
    font-size: 14px;
    font-weight: 700;
    letter-spacing: 0.1em;
  }
  .badge-edge { background: rgba(15, 118, 110, 0.1); color: #0F766E; }
  .badge-cloud { background: rgba(204, 90, 1, 0.1); color: #CC5A01; }
  .badge-end { background: rgba(71, 68, 62, 0.1); color: #47443E; }
  
  .state-title {
    font-size: 26px;
    font-weight: 800;
    color: var(--text-primary);
    margin-bottom: 24px;
    line-height: 1.4;
    min-height: 72px; /* Align titles */
  }
  .state-desc {
    font-size: 18px;
    color: var(--text-secondary);
    line-height: 1.7;
    flex-grow: 1;
    display: flex;
    flex-direction: column;
    gap: 16px;
  }
  .state-desc strong {
    color: var(--text-primary);
    font-size: 19px;
  }
  /* Decoration line */
  .state-card::after {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 6px;
    background: var(--brand-gradient);
    opacity: 0;
    transition: opacity 0.3s ease;
  }
  .state-card:hover::after {
    opacity: 1;
  }
</style>'''

c = c[:old_style_start] + new_style + c[old_style_end:]

old_layout_start = c.find('<div class="layout-box">')
old_layout_end = c.find('<!-- "查看视频演示" Button at Bottom Right -->')

new_layout = '''<div class="three-col-layout">
    <!-- Card 1: Edge (边) -->
    <div class="state-card">
      <div class="state-icon-area">
        <div class="state-num">01</div>
        <div class="state-badge badge-edge">EDGE NODE</div>
      </div>
      <h3 class="state-title">边缘侧状态<br>网络自愈与时钟同步</h3>
      <div class="state-desc">
        <div>
          <strong><code class="proto-tag">ESP8266</code> 自动断网自愈</strong><br>
          通过 <code class="proto-tag">AT</code> 指令进行底层握手，设备断线后自动保存配置并尝试重连，保证边缘节点永远在线。
        </div>
        <div>
          <strong><code class="proto-tag">NTP</code> 毫秒级精准同步</strong><br>
          上电拉取时间；掉线期间系统无缝切入本地 10ms 软件 <code class="proto-tag">RTC</code> 累加，恢复网络后自动对齐误差。
        </div>
      </div>
    </div>

    <!-- Card 2: Cloud (云) -->
    <div class="state-card">
      <div class="state-icon-area">
        <div class="state-num">02</div>
        <div class="state-badge badge-cloud">CLOUD API</div>
      </div>
      <h3 class="state-title">云端侧通信<br>API 解析与警报推送</h3>
      <div class="state-desc">
        <div>
          <strong>心知天气 <code class="proto-tag">API</code> 轻量解析</strong><br>
          直接使用 <code>strstr</code> 从 <code class="proto-tag">TCP</code> 接收环形缓存中匹配键值对，舍弃开销巨大的 <code>JSON</code> 库，极大节约了 RAM 空间。
        </div>
        <div>
          <strong>微信警报与冷却防抖</strong><br>
          遭入侵或温度突变时，调用 Server酱 接口推送微信警报。设计 <strong>180秒发送冷却期</strong> 防范 HTTP 拥塞和频繁重发。
        </div>
      </div>
    </div>

    <!-- Card 3: End (端) -->
    <div class="state-card">
      <div class="state-icon-area">
        <div class="state-num">03</div>
        <div class="state-badge badge-end">END STORAGE</div>
      </div>
      <h3 class="state-title">终端侧归档<br>SD 卡日志持久化</h3>
      <div class="state-desc">
        <div>
          <strong><code class="proto-tag">512B</code> 扇区物理对齐</strong><br>
          基于 FAT32 底层设计的 SD 日志机制，读取或覆写时严格按照物理 512 字节进行对齐擦写，提升存储寿命与速度。
        </div>
        <div>
          <strong>时间戳异常事件追溯</strong><br>
          将含有北京时间戳的入侵事件、系统报错及环境参数实时持久化归档，为后续离线故障排查提供数据追踪。
        </div>
      </div>
    </div>
  </div>
  
  '''

c = c[:old_layout_start] + new_layout + c[old_layout_end:]

open(path, 'w', encoding='utf-8', newline='\n').write(c)

print("SUCCESS: Transformed to 3-column layout")

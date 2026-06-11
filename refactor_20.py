# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/slides/section6-03-wechat-logging.html')[0]
c = open(path, encoding='utf-8').read()

old_style_start = c.find('<style>')
old_style_end = c.find('</style>') + len('</style>')

new_style = '''<style>
  .layout-container {
    display: flex;
    flex-direction: column;
    width: 100%;
    height: 100%;
  }
  .four-col-layout {
    display: flex;
    gap: 24px;
    width: 100%;
    flex-grow: 1;
    margin-top: 10px;
    padding-bottom: 20px;
  }
  .state-card {
    flex: 1;
    background: var(--card-bg);
    border: 1px solid var(--card-border);
    box-shadow: var(--glass-shadow);
    border-radius: 20px;
    padding: 30px 24px;
    display: flex;
    flex-direction: column;
    position: relative;
    overflow: hidden;
    transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);
  }
  .state-card:hover {
    transform: translateY(-8px);
    box-shadow: 0 25px 50px -12px rgba(15, 118, 110, 0.15);
  }
  
  /* Top borders for visual distinction */
  .card-edge { border-top: 6px solid #0F766E; }
  .card-cloud { border-top: 6px solid #0369a1; }
  .card-wechat { border-top: 6px solid #047857; }
  .card-sd { border-top: 6px solid #CC5A01; }
  
  .state-icon-area {
    display: flex;
    align-items: center;
    margin-bottom: 16px;
  }
  .state-num {
    font-size: 42px;
    font-weight: 900;
    background: var(--brand-gradient);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
    font-family: var(--font-title-family);
    width: fit-content;
  }
  
  .state-condition {
    font-size: 22px;
    font-weight: 800;
    color: var(--text-primary);
    margin-bottom: 16px;
    padding-bottom: 12px;
    border-bottom: 1px dashed rgba(25, 24, 22, 0.15);
    min-height: 56px;
    line-height: 1.3;
  }
  .state-desc {
    font-size: 20px;
    color: var(--text-secondary);
    line-height: 1.7;
    flex-grow: 1;
    margin-bottom: 24px;
  }
  .state-desc strong {
    color: var(--text-primary);
    font-weight: 800;
  }
  
  /* Inline button for cards */
  .card-video-btn {
    margin-top: auto;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    padding: 12px 16px;
    background: var(--brand-primary);
    border: 1px solid rgba(25, 24, 22, 0.1);
    border-radius: 10px;
    color: #fff;
    font-size: 16px;
    font-weight: 800;
    cursor: pointer;
    transition: all 0.2s;
    box-shadow: 0 4px 10px rgba(204, 90, 1, 0.15);
  }
  .card-video-btn:hover {
    background: var(--brand-secondary);
    transform: scale(1.02);
    box-shadow: 0 8px 20px rgba(15, 118, 110, 0.2);
  }
  .play-icon { font-size: 16px; }

  /* Fullscreen Video Modal */
  .video-modal {
    position: fixed;
    top: 0; left: 0; width: 100%; height: 100%;
    background: rgba(25, 24, 22, 0.96);
    backdrop-filter: blur(16px);
    z-index: 9999;
    display: none;
    align-items: center;
    justify-content: center;
  }
  .video-modal-content {
    position: relative;
    width: 85%;
    max-width: 1400px;
    aspect-ratio: 16 / 9;
    border-radius: 24px;
    overflow: hidden;
    border: 1px solid var(--card-border);
    box-shadow: 0 25px 60px rgba(25, 24, 22, 0.5);
  }
  .video-modal-content video {
    width: 100%;
    height: 100%;
    display: block;
    outline: none;
  }
  .close-modal-btn {
    position: absolute;
    top: 32px;
    right: 32px;
    width: 54px;
    height: 54px;
    border-radius: 50%;
    background: rgba(252, 250, 247, 0.1);
    border: 1px solid rgba(252, 250, 247, 0.15);
    color: #FCFAF7;
    font-size: 32px;
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    transition: all 0.2s ease;
    z-index: 10000;
  }
  .close-modal-btn:hover {
    background: var(--alert-red);
    border-color: var(--alert-red);
    transform: scale(1.1);
  }
</style>'''

c = c[:old_style_start] + new_style + c[old_style_end:]

old_layout_start = c.find('<div class="layout-box">')
old_layout_end = c.find('<!-- Video Modal -->')

new_layout = '''<div class="layout-container">
    <div class="four-col-layout">
      <!-- Col 1 -->
      <div class="state-card card-edge">
        <div class="state-icon-area">
          <div class="state-num">01</div>
        </div>
        <div class="state-condition"><code class="proto-tag">ESP8266</code> 自愈<br>与 <code class="proto-tag">NTP</code> 同步</div>
        <div class="state-desc">
          通过 <code class="proto-tag">AT</code> 指令进行底层握手，断线后自动保存配置重连。<br><br>掉线期间无缝切入本地 10ms 软件 <code class="proto-tag">RTC</code> 累加，恢复网络后自动对齐秒级误差。
        </div>
        <button class="card-video-btn"><span class="play-icon">▶</span> 查看视频演示</button>
      </div>

      <!-- Col 2 -->
      <div class="state-card card-cloud">
        <div class="state-icon-area">
          <div class="state-num">02</div>
        </div>
        <div class="state-condition">心知天气 <code class="proto-tag">API</code><br>轻量解析</div>
        <div class="state-desc">
          直接使用 <code>strstr</code> 从 <code class="proto-tag">TCP</code> 接收环形缓存中匹配特定键值对。<br><br>舍弃开销巨大的 <code>JSON</code> 库，极大节约了 RAM 空间。
        </div>
        <button class="card-video-btn"><span class="play-icon">▶</span> 查看视频演示</button>
      </div>

      <!-- Col 3 -->
      <div class="state-card card-wechat">
        <div class="state-icon-area">
          <div class="state-num">03</div>
        </div>
        <div class="state-condition">微信警报推送<br>与防抖限制</div>
        <div class="state-desc">
          遭入侵或温度突变偏差 &gt; 5℃ 时，调用 Server酱 向微信推送报警。<br><br>为防 HTTP 拥塞和频繁重入，设计了 <strong>180秒发送冷却期</strong>。
        </div>
        <button class="card-video-btn"><span class="play-icon">▶</span> 查看视频演示</button>
      </div>

      <!-- Col 4 -->
      <div class="state-card card-sd">
        <div class="state-icon-area">
          <div class="state-num">04</div>
        </div>
        <div class="state-condition"><code class="proto-tag">512B</code> 扇区对齐<br><code class="proto-tag">SD</code> 卡日志归档</div>
        <div class="state-desc">
          设计了 SD 日志扇区对齐算法。<br><br>读取或覆写时按照物理 512 字节进行对齐擦写，将含有北京时间戳的事件及环境参数实时持久化。
        </div>
        <button class="card-video-btn"><span class="play-icon">▶</span> 查看视频演示</button>
      </div>
    </div>
  </div>
</div>

<div class="slide-footer">
  <div class="section-name">06. 云边端物联网通信与 SD 日志归档</div>
  <div class="slide-number">Page 20 / 25</div>
</div>

'''

c = c[:old_layout_start] + new_layout + c[old_layout_end:]

# Replace Javascript logic for multiple buttons
old_script_start = c.find('<script>')
old_script_end = c.find('</script>') + len('</script>')

new_script = '''<script>
  const openModalBtns = document.querySelectorAll(".card-video-btn");
  const videoModal = document.getElementById("videoModal");
  const closeModalBtn = document.getElementById("closeModalBtn");
  const modalVideo = document.getElementById("modalVideo");

  openModalBtns.forEach(btn => {
    btn.addEventListener("click", () => {
      videoModal.style.display = "flex";
      modalVideo.play().catch(err => {
        console.log("Modal video autoplay failed or file not found:", err);
      });
    });
  });

  function closeModal() {
    videoModal.style.display = "none";
    modalVideo.pause();
  }

  closeModalBtn.addEventListener("click", closeModal);
  videoModal.addEventListener("click", (e) => {
    if (e.target === videoModal) closeModal();
  });
</script>'''

c = c[:old_script_start] + new_script + c[old_script_end:]

# Wait, we need to fix the page number back to 20 / 25!
# Earlier I noticed it was 19 / 24.
# Let's globally replace 19 / 24 with 20 / 25.
c = c.replace('19 / 24', '20 / 25')

open(path, 'w', encoding='utf-8', newline='\n').write(c)

print("SUCCESS: Transformed slide 20 to 4-column layout with buttons")

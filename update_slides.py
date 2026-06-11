import glob
import os
import shutil
import re

# Find the slides directory
index_path = glob.glob('*文档/index.html')
if not index_path:
    print("Could not find index.html")
    exit(1)
index_path = index_path[0]
slides_dir = os.path.join(os.path.dirname(index_path), 'slides')

# 1. Update index.html
with open(index_path, 'r', encoding='utf-8') as f:
    idx_content = f.read()

# Replace the specific slide entries
idx_content = idx_content.replace(
    '{ file: "slides/section5-03-hmi-controls.html",      label: "15 多通道人机交互控制方式设计" },',
    '{ file: "slides/section5-03-hud-pages-overview.html",label: "15 HUD 六大核心功能页面全览" },\n    { file: "slides/section5-04-hmi-controls.html",      label: "16 多通道人机交互：实体/触屏/红外遥控" },'
)
idx_content = idx_content.replace(
    '{ file: "slides/section5-04-dirty-rects.html",       label: "16 LCD 图形渲染优化与增量更新算法" },',
    '{ file: "slides/section5-05-dirty-rects.html",       label: "17 LCD 图形渲染优化与增量更新算法" },'
)

# Increment subsequent labels (from 17 up to 25)
for i in range(17, 26)[::-1]:
    idx_content = re.sub(rf'label: "{i} ', f'label: "{i+1} ', idx_content)
    # Also fix the cover slide count if it exists like "Page 15 / 24" (Though this is in individual files)

with open(index_path, 'w', encoding='utf-8') as f:
    f.write(idx_content)

# 2. Rename dirty rects
old_dirty = os.path.join(slides_dir, 'section5-04-dirty-rects.html')
new_dirty = os.path.join(slides_dir, 'section5-05-dirty-rects.html')
if os.path.exists(old_dirty):
    shutil.move(old_dirty, new_dirty)

# Fix the page number in the renamed dirty rects
with open(new_dirty, 'r', encoding='utf-8') as f:
    dirty_content = f.read()
dirty_content = dirty_content.replace('Page 16 / 24', 'Page 17 / 25')
with open(new_dirty, 'w', encoding='utf-8') as f:
    f.write(dirty_content)

# 3. Create Slide 1 (Overview of 6 pages)
# Read the original controls HTML which has the LCD DOM
hmi_path = os.path.join(slides_dir, 'section5-03-hmi-controls.html')
with open(hmi_path, 'r', encoding='utf-8') as f:
    hmi_content = f.read()

# Extract the header and styling up to <div class="sandbox-container">
head_match = re.search(r'(.*?<div class="slide-content-layout[^>]*>)', hmi_content, re.DOTALL)
head_html = head_match.group(1)

# Modify title
head_html = head_html.replace('多通道人机交互与<span class="highlight-gradient">网页虚拟仿真大屏</span>', 'HUD 六大核心功能<span class="highlight-gradient">页面全览</span>')
head_html = head_html.replace('Multi-Channel Human-Machine Interface and Real-Time Sandbox Simulation', 'Complete Overview of the 6 Core TFT-LCD Interface Pages')
head_html = head_html.replace('15 多通道人机交互控制方式设计', '15 HUD 六大核心功能页面全览')
head_html = head_html.replace('<title>09 多通道人机交互', '<title>15 HUD 六大核心功能')

# Extract just the <div class="lcd-enclosure">
lcd_match = re.search(r'(<div class="lcd-enclosure">.*?</div>\s*</div>\s*<!-- End of LCD -->|(<div class="lcd-enclosure">.*?)</div>\s*</div>)', hmi_content, re.DOTALL)
if lcd_match:
    lcd_html = lcd_match.group(1)
    # The regex might grab too much or too little, let's just do a string search
    start = hmi_content.find('<div class="lcd-enclosure">')
    # Find matching closing div for lcd-enclosure
    end = start
    depth = 0
    for i in range(start, len(hmi_content)):
        if hmi_content[i:i+4] == '<div':
            depth += 1
        elif hmi_content[i:i+5] == '</div':
            depth -= 1
            if depth == 0:
                end = i + 6
                break
    lcd_html = hmi_content[start:end]
else:
    lcd_html = "ERROR"

overview_html = head_html + """
  <div style="width: 100%; height: 100%; display: flex; justify-content: center; align-items: center; overflow: hidden;">
    <!-- Scale down the grid to fit the slide -->
    <div style="transform: scale(0.48); transform-origin: center center; width: 1900px; height: 950px; display: grid; grid-template-columns: repeat(3, 1fr); gap: 50px; padding: 20px;" id="grid-container">
    </div>
  </div>
  
  <div id="source-lcd" style="display:none;">
""" + lcd_html + """
  </div>
</div>

<div class="slide-footer">
  <div class="section-name">05. HUD UI 设计与高保真仿真展示</div>
  <div class="slide-number">Page 15 / 25</div>
</div>

<script>
  const pageTitles = [
    "[1] REAL-TIME MONITORING",
    "[2] AI BASELINE LEARNING",
    "[3] ANOMALY HISTORY LOGS",
    "[4] FLAPPY BIRD MINI-GAME",
    "[5] SECURITY GUARD MODE",
    "[6] IOT WEATHER & CLOCK"
  ];
  const source = document.getElementById('source-lcd').children[0];
  const grid = document.getElementById('grid-container');
  
  for(let i=0; i<6; i++) {
      let clone = source.cloneNode(true);
      clone.id = "cloned-lcd-" + i;
      // hide all p-tabs except the current one
      let tabs = clone.querySelectorAll('.p-tab');
      tabs.forEach((tab, idx) => {
          if(idx === i) tab.classList.add('active');
          else tab.classList.remove('active');
      });
      clone.querySelector('.lcd-header-title').textContent = pageTitles[i];
      let pill = clone.querySelector('.lcd-topbar-pill');
      if(pill) pill.textContent = "SYSTEM SAFE";
      
      let pageLbl = clone.querySelector('#lcdPageName') || clone.querySelector('.lcd-header-title').previousElementSibling;
      if(pageLbl && pageLbl.id === 'lcdPageName') pageLbl.textContent = "PAGE " + i;
      
      let botRight = clone.querySelector('.lcd-bottom-right');
      if(botRight) botRight.textContent = "G-05 HUD PAGE " + i;
      
      // highlight dots
      let dots = clone.querySelectorAll('.p-dot');
      dots.forEach((dot, idx) => {
          if(idx === i) dot.classList.add('active');
          else dot.classList.remove('active');
      });
      
      // Initialize some bars
      if (i === 0) {
        let b1 = clone.querySelector('#p0-temp-bar');
        let b2 = clone.querySelector('#p0-humi-bar');
        if(b1) b1.innerHTML = Array(12).fill(0).map((_, j) => `<div class="seg-block ${j<6?'active':'inactive'}" style="background:var(--lcd-accent)"></div>`).join('');
        if(b2) b2.innerHTML = Array(12).fill(0).map((_, j) => `<div class="seg-block ${j<6?'active':'inactive'}" style="background:var(--lcd-accent)"></div>`).join('');
      } else if (i === 1) {
        let b = clone.querySelector('#p1-progress-bar');
        if(b) b.innerHTML = Array(12).fill(0).map((_, j) => `<div class="seg-block ${j<10?'active':'inactive'}" style="background:var(--lcd-green)"></div>`).join('');
      } else if (i === 4) {
        let b = clone.querySelector('#p4-dist-bar');
        if(b) b.innerHTML = Array(12).fill(0).map((_, j) => `<div class="seg-block ${j<2?'active':'inactive'}" style="background:var(--lcd-yellow)"></div>`).join('');
      }
      
      grid.appendChild(clone);
  }
</script>
</body>
</html>
"""

with open(os.path.join(slides_dir, 'section5-03-hud-pages-overview.html'), 'w', encoding='utf-8') as f:
    f.write(overview_html)


# 4. Create Slide 2 (3 Control Modes)
# We can use the head_html from hmi_content as base
head_html_2 = head_match.group(1)
head_html_2 = head_html_2.replace('多通道人机交互与<span class="highlight-gradient">网页虚拟仿真大屏</span>', '多通道人机交互：<span class="highlight-gradient">全方位控制方案</span>')
head_html_2 = head_html_2.replace('Multi-Channel Human-Machine Interface and Real-Time Sandbox Simulation', 'Multi-Channel HMI: Physical Buttons, Touch Screen, and IR Remote')
head_html_2 = head_html_2.replace('15 多通道人机交互控制方式设计', '16 多通道人机交互：实体/触屏/红外')

controls_html = head_html_2 + """
  <div style="display: flex; justify-content: center; align-items: stretch; height: 100%; padding: 40px 60px; gap: 40px;">
    
    <!-- 1. 实体按键 -->
    <div style="flex: 1; background: #FCFAF7; border: 1px solid #EAE6DF; border-radius: 20px; padding: 40px; text-align: center; box-shadow: 0 15px 35px rgba(25,24,22,0.08); display: flex; flex-direction: column;">
      <div style="height: 160px; display: flex; align-items: center; justify-content: center; font-size: 80px;">🕹️</div>
      <h2 style="color: var(--brand-primary); margin-bottom: 24px; font-size: 28px;">1. 实验箱物理按键</h2>
      <p style="color: var(--text-muted); font-size: 18px; line-height: 1.8; text-align: left; background: #F3EBE1; padding: 20px; border-radius: 12px; flex-grow: 1;">
        <strong style="color: #4A453E;">输入源：</strong><br>实验板载独立按键 (KEY1-KEY4) + 五向导航摇杆<br><br>
        <strong style="color: #4A453E;">特点：</strong><br>
        提供最稳定、零延迟的硬件级控制，带有物理段落感。<br>适用于基础的菜单切换、系统急停复位及游戏控制。
      </p>
    </div>

    <!-- 2. 触摸屏 -->
    <div style="flex: 1; background: #FCFAF7; border: 1px solid #EAE6DF; border-radius: 20px; padding: 40px; text-align: center; box-shadow: 0 15px 35px rgba(25,24,22,0.08); display: flex; flex-direction: column;">
      <div style="height: 160px; display: flex; align-items: center; justify-content: center; font-size: 80px;">👆</div>
      <h2 style="color: var(--brand-primary); margin-bottom: 24px; font-size: 28px;">2. TFT 电阻触摸屏</h2>
      <p style="color: var(--text-muted); font-size: 18px; line-height: 1.8; text-align: left; background: #F3EBE1; padding: 20px; border-radius: 12px; flex-grow: 1;">
        <strong style="color: #4A453E;">输入源：</strong><br>XPT2046 触摸控制器采集压感坐标<br><br>
        <strong style="color: #4A453E;">特点：</strong><br>
        实现直观的图形化 GUI 交互。用户可直接点击屏幕虚拟按钮或进行交互操作，体验类似现代智能手机的直觉控制。
      </p>
    </div>

    <!-- 3. 红外遥控 -->
    <div style="flex: 1; background: #FCFAF7; border: 1px solid #EAE6DF; border-radius: 20px; padding: 40px; text-align: center; box-shadow: 0 15px 35px rgba(25,24,22,0.08); display: flex; flex-direction: column;">
      <div style="height: 160px; display: flex; align-items: center; justify-content: center;">
        <img src="../images/remote.jpg" alt="红外遥控器图片" style="max-height: 140px; border-radius: 12px; box-shadow: 0 8px 24px rgba(0,0,0,0.15);" onerror="this.onerror=null; this.src=''; this.alt='[ 请将照片保存为: 展示文档/images/remote.jpg ]'; this.style.fontSize='16px'; this.style.border='2px dashed #CC5A01'; this.style.padding='20px'; this.style.boxShadow='none';">
      </div>
      <h2 style="color: var(--brand-primary); margin-bottom: 24px; font-size: 28px;">3. NEC 红外遥控器</h2>
      <p style="color: var(--text-muted); font-size: 18px; line-height: 1.8; text-align: left; background: #F3EBE1; padding: 20px; border-radius: 12px; flex-grow: 1;">
        <strong style="color: #4A453E;">输入源：</strong><br>17 键红外遥控器 + EXTI 外部中断 + TIM4<br><br>
        <strong style="color: #4A453E;">特点：</strong><br>
        实现远距离无线操控。支持翻页、布撤防、主题切换等高频操作。采用标准 NEC 协议严格解码，抗干扰性强。
      </p>
    </div>

  </div>
</div>

<div class="slide-footer">
  <div class="section-name">05. HUD UI 设计与人机交互</div>
  <div class="slide-number">Page 16 / 25</div>
</div>
</body>
</html>
"""

with open(os.path.join(slides_dir, 'section5-04-hmi-controls.html'), 'w', encoding='utf-8') as f:
    f.write(controls_html)

# Delete the old hmi controls if it's there
os.remove(hmi_path)

print("SUCCESS")

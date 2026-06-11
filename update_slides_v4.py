# -*- coding: utf-8 -*-
import os
import re
import glob

index_path = glob.glob('*文档/index.html')
if not index_path:
    print("Could not find index.html")
    exit(1)
slides_dir = os.path.join(os.path.dirname(index_path[0]), 'slides')

overview_path = os.path.join(slides_dir, 'section5-03-hud-pages-overview.html')
controls_path = os.path.join(slides_dir, 'section5-04-hmi-controls.html')

with open('temp_slide.txt', 'r', encoding='utf-8') as f:
    temp_content = f.read()

# --- Rebuild Slide 15 (Overview) ---
head_match = re.search(r'(.*?<div class="slide-content-layout[^>]*>)', temp_content, re.DOTALL)
head_html = head_match.group(1)

head_html = head_html.replace('多通道人机交互与<span class="highlight-gradient">网页虚拟仿真大屏</span>', 'HUD 六大核心功能<span class="highlight-gradient">页面全览</span>')
head_html = head_html.replace('Multi-Channel Human-Machine Interface and Real-Time Sandbox Simulation', 'Complete Overview of the 6 Core TFT-LCD Interface Pages')
head_html = head_html.replace('15 多通道人机交互控制方式设计', '15 HUD 六大核心功能页面全览')
head_html = head_html.replace('<title>09 多通道人机交互', '<title>15 HUD 六大核心功能')

start = temp_content.find('<div class="lcd-enclosure">')
end = start
depth = 0
for i in range(start, len(temp_content)):
    if temp_content[i:i+4] == '<div':
        depth += 1
    elif temp_content[i:i+5] == '</div':
        depth -= 1
        if depth == 0:
            end = i + 6
            break
lcd_html = temp_content[start:end]

overview_html = head_html + """
  <div style="width: 100%; height: 100%; display: flex; justify-content: center; align-items: flex-start; overflow: hidden; padding-top: 10px;">
    <!-- Scale to fit beautifully -->
    <div style="transform: scale(0.68); transform-origin: center top; width: 1950px; display: grid; grid-template-columns: repeat(3, 1fr); gap: 60px 40px; justify-items: center;" id="grid-container">
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
  
  const bottomLabels = [
    "1. 实时温湿度监控主页",
    "2. 异常判定 AI 自学习",
    "3. 异常历史归档日志",
    "4. 飞鸟小游戏沉浸体验",
    "5. 居家防盗安防模式",
    "6. 物联网实时天气时钟"
  ];

  const source = document.getElementById('source-lcd').children[0];
  const grid = document.getElementById('grid-container');
  
  for(let i=0; i<6; i++) {
      let wrapper = document.createElement('div');
      wrapper.style.display = 'flex';
      wrapper.style.flexDirection = 'column';
      wrapper.style.alignItems = 'center';
      wrapper.style.gap = '24px';
      
      let clone = source.cloneNode(true);
      clone.id = "cloned-lcd-" + i;
      
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
      
      let dots = clone.querySelectorAll('.p-dot');
      dots.forEach((dot, idx) => {
          if(idx === i) dot.classList.add('active');
          else dot.classList.remove('active');
      });
      
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
      
      let label = document.createElement('div');
      label.textContent = bottomLabels[i];
      label.style.fontSize = '38px';
      label.style.fontWeight = 'bold';
      label.style.color = '#0F766E';
      label.style.letterSpacing = '2px';
      
      wrapper.appendChild(clone);
      wrapper.appendChild(label);
      grid.appendChild(wrapper);
  }
</script>
</body>
</html>
"""

with open(overview_path, 'w', encoding='utf-8') as f:
    f.write(overview_html)

# --- Rebuild Slide 16 (Controls) ---
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
      <button style="margin-top: 20px; padding: 12px 24px; background: #0F766E; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; box-shadow: 0 4px 12px rgba(15,118,110,0.3); transition: all 0.2s;" onmouseover="this.style.background='#115E59'; this.style.transform='translateY(-2px)'" onmouseout="this.style.background='#0F766E'; this.style.transform='none'">▶ 播放按键操控演示</button>
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
      <button style="margin-top: 20px; padding: 12px 24px; background: #0F766E; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; box-shadow: 0 4px 12px rgba(15,118,110,0.3); transition: all 0.2s;" onmouseover="this.style.background='#115E59'; this.style.transform='translateY(-2px)'" onmouseout="this.style.background='#0F766E'; this.style.transform='none'">▶ 播放触屏操控演示</button>
    </div>

    <!-- 3. 红外遥控 -->
    <div style="flex: 1; background: #FCFAF7; border: 1px solid #EAE6DF; border-radius: 20px; padding: 40px; text-align: center; box-shadow: 0 15px 35px rgba(25,24,22,0.08); display: flex; flex-direction: column;">
      <div style="height: 160px; display: flex; align-items: center; justify-content: center;">
        
        <!-- Authentic 17-key Mini Remote HTML -->
        <div style="width: 76px; height: 154px; background: #181818; border-radius: 6px; border: 2px solid #333; display: flex; flex-direction: column; align-items: center; padding: 10px 8px; box-shadow: 0 8px 16px rgba(0,0,0,0.4), inset 0 1px 3px rgba(255,255,255,0.1);">
          <div style="width: 100%; display: grid; grid-template-columns: repeat(3, 1fr); gap: 6px 4px; margin-bottom: 12px;">
            <!-- Row 1 -->
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">1</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">2</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">3</div>
            <!-- Row 2 -->
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">4</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">5</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">6</div>
            <!-- Row 3 -->
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">7</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">8</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">9</div>
            <!-- Row 4 -->
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">*</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">0</div>
            <div style="aspect-ratio: 1; background: #383838; border-radius: 50%; color: #eee; font-size: 9px; font-family: sans-serif; display: flex; align-items: center; justify-content: center; box-shadow: 0 2px 2px rgba(0,0,0,0.5);">#</div>
          </div>
          <!-- Navigation Pad -->
          <div style="position: relative; width: 48px; height: 48px; background: #252525; border-radius: 50%; box-shadow: inset 0 0 4px rgba(0,0,0,0.8);">
            <!-- Up -->
            <div style="position: absolute; top: 2px; left: 16px; width: 16px; height: 16px; background: #383838; border-radius: 50%; color: #ccc; font-size: 7px; display: flex; align-items: center; justify-content: center; box-shadow: 0 1px 1px rgba(0,0,0,0.5);">▲</div>
            <!-- Down -->
            <div style="position: absolute; bottom: 2px; left: 16px; width: 16px; height: 16px; background: #383838; border-radius: 50%; color: #ccc; font-size: 7px; display: flex; align-items: center; justify-content: center; box-shadow: 0 1px 1px rgba(0,0,0,0.5);">▼</div>
            <!-- Left -->
            <div style="position: absolute; left: 2px; top: 16px; width: 16px; height: 16px; background: #383838; border-radius: 50%; color: #ccc; font-size: 7px; display: flex; align-items: center; justify-content: center; box-shadow: 0 1px 1px rgba(0,0,0,0.5);">◀</div>
            <!-- Right -->
            <div style="position: absolute; right: 2px; top: 16px; width: 16px; height: 16px; background: #383838; border-radius: 50%; color: #ccc; font-size: 7px; display: flex; align-items: center; justify-content: center; box-shadow: 0 1px 1px rgba(0,0,0,0.5);">▶</div>
            <!-- OK -->
            <div style="position: absolute; left: 16px; top: 16px; width: 16px; height: 16px; background: #0F766E; border-radius: 50%; color: white; font-size: 6px; font-weight: bold; display: flex; align-items: center; justify-content: center; box-shadow: 0 1px 2px rgba(0,0,0,0.6);">OK</div>
          </div>
        </div>

      </div>
      <h2 style="color: var(--brand-primary); margin-bottom: 24px; font-size: 28px;">3. NEC 红外遥控器</h2>
      <p style="color: var(--text-muted); font-size: 18px; line-height: 1.8; text-align: left; background: #F3EBE1; padding: 20px; border-radius: 12px; flex-grow: 1;">
        <strong style="color: #4A453E;">输入源：</strong><br>17 键红外遥控器 + EXTI 外部中断 + TIM4<br><br>
        <strong style="color: #4A453E;">特点：</strong><br>
        实现远距离无线操控。支持翻页、布撤防、主题切换等高频操作。采用标准 NEC 协议严格解码，抗干扰性强。
      </p>
      <button style="margin-top: 20px; padding: 12px 24px; background: #0F766E; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; box-shadow: 0 4px 12px rgba(15,118,110,0.3); transition: all 0.2s;" onmouseover="this.style.background='#115E59'; this.style.transform='translateY(-2px)'" onmouseout="this.style.background='#0F766E'; this.style.transform='none'">▶ 播放红外遥控演示</button>
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

with open(controls_path, 'w', encoding='utf-8') as f:
    f.write(controls_html)

print("SUCCESS")

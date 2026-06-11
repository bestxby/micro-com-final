# -*- coding: utf-8 -*-
import glob, re

slide17_path = glob.glob('*文档/slides/section5-03-dirty-rects.html')[0]

with open(slide17_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Remove old animation CSS
css_start = content.find('/* Mock LCD refresh visuals */')
css_end = content.find('  p, li { font-size: 23px')

new_css = """  /* High-fidelity LCD refresh simulation */
  .monitor-box {
    width: 320px;
    height: 240px;
    background: #0f172a;
    border: 4px solid #334155;
    border-radius: 12px;
    position: relative;
    overflow: hidden;
    box-shadow: inset 0 0 20px rgba(0,0,0,0.8), 0 10px 30px rgba(0,0,0,0.3);
  }
  .bg-grid {
    position: absolute; width: 100%; height: 100%;
    background-image: linear-gradient(rgba(255,255,255,0.05) 1px, transparent 1px),
                      linear-gradient(90deg, rgba(255,255,255,0.05) 1px, transparent 1px);
    background-size: 20px 20px;
  }
  .demo-container {
    display: flex; flex-direction: column; align-items: center; gap: 15px;
  }
  .mock-object {
    color: white; font-size: 12px; font-weight: bold;
    display: flex; align-items: center; justify-content: center;
    border-radius: 4px; position: absolute;
    box-shadow: 0 4px 10px rgba(0,0,0,0.5);
  }
  .stutter-bird, .smooth-bird { width: 40px; height: 30px; background: #eab308; border: 2px solid #fef08a; }
  .stutter-pipe, .smooth-pipe { width: 50px; height: 120px; background: #22c55e; border: 2px solid #86efac; }

  /* Stutter animations for Full Redraw */
  .stutter-bird { top: 100px; left: 60px; animation: stutterBird 2s infinite steps(4); }
  @keyframes stutterBird {
    0% { transform: translateY(0); }
    50% { transform: translateY(-35px); }
    100% { transform: translateY(0); }
  }
  .stutter-pipe { bottom: 0; right: 20px; animation: stutterPipe 3s infinite steps(5); }
  @keyframes stutterPipe {
    0% { transform: translateX(0); }
    100% { transform: translateX(-80px); }
  }

  /* Smooth animations for Dirty Rects */
  .smooth-bird-container {
    position: absolute; top: 100px; left: 60px;
    width: 40px; height: 30px;
    animation: smoothBird 1s infinite alternate ease-in-out;
  }
  @keyframes smoothBird {
    0% { transform: translateY(0); }
    100% { transform: translateY(-35px); }
  }
  .smooth-bird { top:0; left:0; width:100%; height:100%; }

  .smooth-pipe-container {
    position: absolute; bottom: 0; right: 20px;
    width: 50px; height: 120px;
    animation: smoothPipe 3s infinite linear;
  }
  @keyframes smoothPipe {
    0% { transform: translateX(0); }
    100% { transform: translateX(-150px); }
  }
  .smooth-pipe { top:0; left:0; width:100%; height:100%; }

  /* The Full Sweep Scanline */
  .red-scanline {
    position: absolute; top: 0; left: 0; width: 100%; height: 6px;
    background: #ef4444; box-shadow: 0 0 15px 5px rgba(239, 68, 68, 0.6);
    animation: scanDown 1.5s infinite linear; z-index: 10;
  }
  .red-flash {
    position: absolute; top: 0; left: 0; width: 100%; height: 100%;
    background: rgba(239, 68, 68, 0.1); animation: flashBg 1.5s infinite linear;
  }
  @keyframes scanDown {
    0% { top: -10px; }
    100% { top: 100%; }
  }
  @keyframes flashBg {
    0%, 100% { opacity: 0.1; }
    50% { opacity: 0.4; }
  }

  /* The Dirty Rect Target Boxes */
  .dirty-rect-target {
    position: absolute; top: -6px; left: -6px; right: -6px; bottom: -6px;
    border: 2px dashed #06b6d4; background: rgba(6, 182, 212, 0.2);
    box-shadow: 0 0 10px rgba(6, 182, 212, 0.5);
    animation: pulseCyan 0.3s infinite alternate; z-index: 5; border-radius: 6px;
  }
  @keyframes pulseCyan {
    0% { opacity: 0.5; }
    100% { opacity: 1; border-color: #22d3ee; }
  }

  .desc-badge {
    position: absolute; bottom: 10px; left: 50%; transform: translateX(-50%);
    font-size: 11px; font-weight: bold; padding: 4px 10px; border-radius: 20px;
    white-space: nowrap; z-index: 20;
  }
  .red-badge { background: rgba(239, 68, 68, 0.2); color: #fca5a5; border: 1px solid rgba(239, 68, 68, 0.4); }
  .cyan-badge { background: rgba(6, 182, 212, 0.2); color: #67e8f9; border: 1px solid rgba(6, 182, 212, 0.4); }

"""

content = content[:css_start] + new_css + content[css_end:]


# Remove old HTML structure and insert new one
html_start = content.find('<!-- Right visual demo -->')
html_end = content.find('</div>\n  </div>\n</div>\n\n<button class="view-video-btn"')

new_html = """<!-- Right visual demo -->
    <div class="right-visual">
      <!-- Full screen redraw -->
      <div class="demo-container">
        <div class="monitor-box full-redraw-monitor">
          <div class="bg-grid"></div>
          
          <div class="stutter-bird mock-object">Bird</div>
          <div class="stutter-pipe mock-object">Pipe</div>
          
          <div class="red-scanline"></div>
          <div class="red-flash"></div>
          
          <div class="desc-badge red-badge">整屏覆写 (频闪/撕裂)</div>
        </div>
        <div class="label-tag">全屏重绘 (2 FPS)</div>
      </div>

      <!-- Dirty Rect redraw -->
      <div class="demo-container">
        <div class="monitor-box dirty-rect-monitor">
          <div class="bg-grid"></div>
          
          <div class="smooth-bird-container">
            <div class="dirty-rect-target"></div>
            <div class="smooth-bird mock-object">Bird</div>
          </div>
          
          <div class="smooth-pipe-container">
            <div class="dirty-rect-target"></div>
            <div class="smooth-pipe mock-object">Pipe</div>
          </div>
          
          <div class="desc-badge cyan-badge">精准定位区域重写</div>
        </div>
        <div class="label-tag">脏矩形增量刷新 (30+ FPS)</div>
      </div>"""

content = content[:html_start] + new_html + content[html_end:]

with open(slide17_path, 'w', encoding='utf-8') as f:
    f.write(content)

# Bump cache to v=8
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        c = file.read()
    c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=8', c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(c)

print("SUCCESS: Refactored animation")

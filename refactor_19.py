# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/slides/section6-02-home-security.html')[0]
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
  .three-col-layout {
    display: flex;
    gap: 30px;
    width: 100%;
    flex-grow: 1;
    margin-top: 10px;
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
  }
  .card-safe { border-top: 6px solid #0F766E; }
  .card-warning { border-top: 6px solid #CC5A01; }
  .card-alarm { border-top: 6px solid #DC2626; }
  
  .state-badge {
    padding: 8px 16px;
    border-radius: 8px;
    font-size: 22px;
    font-weight: 900;
    display: inline-block;
    width: fit-content;
    margin-bottom: 24px;
    letter-spacing: 0.05em;
  }
  .badge-safe { background: rgba(15, 118, 110, 0.1); color: #0F766E; border: 1px solid rgba(15, 118, 110, 0.2); }
  .badge-warning { background: rgba(204, 90, 1, 0.1); color: #CC5A01; border: 1px solid rgba(204, 90, 1, 0.2); }
  .badge-alarm { background: rgba(220, 38, 38, 0.1); color: #DC2626; border: 1px solid rgba(220, 38, 38, 0.2); }
  
  .state-condition {
    font-size: 20px;
    font-weight: 700;
    color: var(--text-primary);
    margin-bottom: 20px;
    padding-bottom: 16px;
    border-bottom: 1px dashed rgba(25, 24, 22, 0.15);
    min-height: 58px;
  }
  .state-desc {
    font-size: 19px;
    color: var(--text-secondary);
    line-height: 1.8;
    flex-grow: 1;
  }
  .state-desc strong {
    color: var(--text-primary);
    font-weight: 800;
  }
  
  .fsm-box {
    background: var(--card-bg);
    border: 1px solid var(--card-border);
    box-shadow: var(--glass-shadow);
    border-radius: 16px;
    padding: 24px 32px;
    margin-top: 24px;
    width: 100%;
  }
</style>'''

c = c[:old_style_start] + new_style + c[old_style_end:]

old_layout_start = c.find('<div class="layout-box">')
old_layout_end = c.find('<!-- "查看视频演示" Button at Bottom Right -->')

new_layout = '''<div class="layout-container">
    <div class="three-col-layout">
      <!-- SAFE -->
      <div class="state-card card-safe">
        <div class="state-badge badge-safe">SAFE 状态</div>
        <div class="state-condition">触发条件：超声波测距 &ge; 30cm 或处于撤防模式</div>
        <div class="state-desc">
          <strong>外设动作：</strong><br><br>
          绿灯 (LED4) 平滑呼吸运行，有源蜂鸣器静音。<br>系统处于常规环境监控状态，安全无风险。
        </div>
      </div>
      
      <!-- WARNING -->
      <div class="state-card card-warning">
        <div class="state-badge badge-warning">WARNING 状态</div>
        <div class="state-condition">触发条件：布防模式下，距离介于 15cm 至 30cm</div>
        <div class="state-desc">
          <strong>外设动作：</strong><br><br>
          黄灯 (LED3) 保持常亮警告，蜂鸣器静音。<br>屏幕数码管开始实时显示侵入距离，提示可能的安全威胁。
        </div>
      </div>
      
      <!-- ALARM -->
      <div class="state-card card-alarm">
        <div class="state-badge badge-alarm">ALARM 状态</div>
        <div class="state-condition">触发条件：布防模式下，距离 &lt; 15cm</div>
        <div class="state-desc">
          <strong>外设动作：</strong><br><br>
          红灯 (LED2) 5Hz 高频爆闪，蜂鸣器断续声鸣警报。<br>屏幕弹出警报窗口，并立即触发 <strong>微信警报推送 (180s 冷却防抖限制)</strong>。
        </div>
      </div>
    </div>

    <div class="fsm-box">
      <h3 style="font-size: 24px; font-weight: 800; color: var(--brand-secondary); margin-bottom: 8px;">状态机转换规则 (FSM)</h3>
      <p style="font-size: 20px; color: var(--text-muted); line-height: 1.6;">
        • 撤防模式 (Disarmed) &rarr; 按 KEY3 / 红外OK &rarr; 布防监测 (Armed)<br>
        • Armed 状态下，系统持续轮询测距：当超声波距离突变触发特定边界时，直接进行 <strong><span style="color: #0F766E;">Safe</span> &harr; <span style="color: #CC5A01;">Warning</span> &harr; <span style="color: #DC2626;">Alarm</span></strong> 的状态机物理映射。撤防操作将强制系统重置为 <strong style="color: #0F766E;">Safe</strong> 状态。
      </p>
    </div>
  </div>
  
  '''

c = c[:old_layout_start] + new_layout + c[old_layout_end:]

open(path, 'w', encoding='utf-8', newline='\n').write(c)

print("SUCCESS: Transformed slide 19 to 3-column layout")

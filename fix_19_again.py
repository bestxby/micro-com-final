# -*- coding: utf-8 -*-
import glob

path = glob.glob('演示文档/slides/section6-02-home-security.html')[0]
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Remove flex-grow: 1 from .three-col-layout
c = c.replace('flex-grow: 1;', '')

# 2. Fix SAFE state desc
old_safe = '''<strong>外设动作：</strong><br><br>
          绿灯 (LED4) 平滑呼吸运行，有源蜂鸣器静音。<br>系统处于常规环境监控状态，安全无风险。'''
new_safe = '''<strong>外设动作：</strong>绿灯 (LED4) 平滑呼吸运行，有源蜂鸣器静音。系统处于常规环境监控状态，安全无风险。'''
c = c.replace(old_safe, new_safe)

# 3. Fix WARNING state desc
old_warn = '''<strong>外设动作：</strong><br><br>
          黄灯 (LED3) 保持常亮警告，蜂鸣器静音。<br>屏幕数码管开始实时显示侵入距离，提示可能的安全威胁。'''
new_warn = '''<strong>外设动作：</strong>黄灯 (LED3) 保持常亮警告，蜂鸣器静音。屏幕数码管开始实时显示侵入距离，提示可能的安全威胁。'''
c = c.replace(old_warn, new_warn)

# 4. Fix ALARM state desc
old_alarm = '''<strong>外设动作：</strong><br><br>
          红灯 (LED2) 5Hz 高频爆闪，蜂鸣器断续声鸣警报。<br>屏幕弹出警报窗口，并立即触发微信警报推送。'''
new_alarm = '''<strong>外设动作：</strong>红灯 (LED2) 5Hz 高频爆闪，蜂鸣器断续声鸣警报。屏幕弹出警报窗口，并立即触发微信警报推送。'''
c = c.replace(old_alarm, new_alarm)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Refactored page 19 texts and removed flex-grow")

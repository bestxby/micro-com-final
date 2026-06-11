# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/slides/section6-02-home-security.html')[0]
c = open(path, encoding='utf-8').read()

# Replace CSS
old_css = '''  .state-condition {
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
  }'''

new_css = '''  .state-condition {
    font-size: 23px;
    font-weight: 700;
    color: var(--text-primary);
    margin-bottom: 20px;
    padding-bottom: 16px;
    border-bottom: 1px dashed rgba(25, 24, 22, 0.15);
    min-height: 72px;
  }
  .state-desc {
    font-size: 23px;
    color: var(--text-secondary);
    line-height: 1.8;
    flex-grow: 1;
  }
  .state-desc strong {
    color: var(--text-primary);
    font-size: 26px;
    font-weight: 800;
  }
  
  .fsm-box {
    background: var(--card-bg);
    border: 1px solid var(--card-border);
    box-shadow: var(--glass-shadow);
    border-radius: 16px;
    padding: 24px 220px 24px 32px; /* Pad right to prevent overlap with video button */
    margin-top: 24px;
    width: 100%;
  }'''
c = c.replace(old_css, new_css)

# Replace HTML inline styles
old_h3 = '<h3 style="font-size: 24px; font-weight: 800; color: var(--brand-secondary); margin-bottom: 8px;">状态机转换规则 (FSM)</h3>'
new_h3 = '<h3 style="font-size: 32px; font-weight: 800; color: var(--brand-secondary); margin-bottom: 12px;">状态机转换规则 (FSM)</h3>'
c = c.replace(old_h3, new_h3)

old_p = '<p style="font-size: 20px; color: var(--text-muted); line-height: 1.6;">'
new_p = '<p style="font-size: 23px; color: var(--text-muted); line-height: 1.8;">'
c = c.replace(old_p, new_p)

open(path, 'w', encoding='utf-8', newline='\n').write(c)

print("SUCCESS: Updated font sizes and padded fsm-box")

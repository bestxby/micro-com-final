# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/slides/section0-02-agenda.html')[0]
c = open(path, encoding='utf-8').read()

old_style = '''<style>
  .agenda-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    grid-template-rows: repeat(3, 1fr);
    gap: 20px;
    width: 100%;
    height: 100%;
    padding: 10px 0;
  }
  .agenda-card {
    cursor: pointer;
    transition: all 0.2s ease;
    display: flex;
    flex-direction: column;
    padding: 24px;
    height: 100%;
    justify-content: center;
    border-radius: 16px;
    position: relative;
    overflow: hidden;
  }
  .agenda-card:nth-child(7) {
    grid-column: span 3;
    flex-direction: row;
    align-items: center;
    justify-content: flex-start;
    gap: 30px;
    padding: 20px 40px;
  }
  .agenda-num {
    font-size: 46px;
    font-weight: 900;
    color: var(--brand-secondary);
    font-family: var(--font-title-family);
    margin-bottom: 6px;
  }
  .agenda-card:nth-child(7) .agenda-num {
    margin-bottom: 0;
    font-size: 54px;
  }
  .agenda-title {
    font-size: 26px;
    font-weight: 800;
    color: var(--text-primary);
    margin-bottom: 6px;
  }
  .agenda-desc {
    font-size: 20px;
    color: var(--text-secondary);
    line-height: 1.5;
  }
  .agenda-card:hover {
    transform: translateY(-4px);
    box-shadow: 0 15px 30px -10px rgba(15, 118, 110, 0.15);
    border-color: rgba(15, 118, 110, 0.3);
  }
</style>'''

new_style = '''<style>
  .agenda-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    grid-template-rows: repeat(3, 1fr);
    gap: 20px;
    width: 100%;
    height: 100%;
    padding: 10px 0;
  }
  .agenda-grid > a {
    display: block;
    text-decoration: none;
    color: inherit;
    outline: none;
    border: none;
    height: 100%;
  }
  .agenda-grid > a:nth-child(7) {
    grid-column: span 3;
  }
  .agenda-card {
    cursor: pointer;
    transition: all 0.2s ease;
    display: flex;
    flex-direction: column;
    padding: 24px;
    height: 100%;
    justify-content: center;
    border-radius: 16px;
    position: relative;
    overflow: hidden;
  }
  .agenda-grid > a:nth-child(7) .agenda-card {
    flex-direction: row;
    align-items: center;
    justify-content: flex-start;
    gap: 30px;
    padding: 20px 40px;
  }
  .agenda-num {
    font-size: 46px;
    font-weight: 900;
    background: var(--brand-gradient);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
    font-family: var(--font-title-family);
    margin-bottom: 6px;
  }
  .agenda-grid > a:nth-child(7) .agenda-num {
    margin-bottom: 0;
    font-size: 54px;
  }
  .agenda-title {
    font-size: 26px;
    font-weight: 800;
    color: var(--text-primary);
    margin-bottom: 6px;
  }
  .agenda-desc {
    font-size: 20px;
    color: var(--text-secondary);
    line-height: 1.5;
  }
  .agenda-card:hover {
    transform: translateY(-4px);
    box-shadow: 0 15px 30px -10px rgba(15, 118, 110, 0.15);
    border-color: rgba(15, 118, 110, 0.3);
  }
</style>'''

c = c.replace(old_style, new_style)
open(path, 'w', encoding='utf-8', newline='\n').write(c)

print("SUCCESS: Updated agenda CSS for colorful numbers and fixed 07 layout.")

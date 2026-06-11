# -*- coding: utf-8 -*-
import glob

path = glob.glob('*文档/index.html')[0]

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

old_open = '''  function openMenu() {
    buildMenu();
    deckMenu.style.display = 'flex';
    menuOverlay.style.display = 'block';
    setTimeout(() => {
      deckMenu.classList.add('open');
      menuOverlay.style.opacity = '1';
    }, 10);
  }'''

new_open = '''  function openMenu() {
    buildMenu();
    // 强制使用内联样式暴击，无视一切外部CSS干扰
    deckMenu.setAttribute("style", "display: flex !important; flex-direction: column !important; opacity: 1 !important; transform: none !important; z-index: 2147483647 !important; position: fixed !important; bottom: 80px !important; right: 24px !important; width: 380px !important; max-height: 500px !important; background: rgba(252, 250, 247, 0.98) !important; border: 1px solid rgba(25, 24, 22, 0.08) !important; border-radius: 16px !important; box-shadow: 0 20px 25px -5px rgba(25,24,22,0.1) !important;");
    menuOverlay.setAttribute("style", "display: block !important; opacity: 1 !important; z-index: 2147483646 !important; background: rgba(25, 24, 22, 0.3) !important; position: fixed !important; top: 0 !important; left: 0 !important; right: 0 !important; bottom: 0 !important;");
    deckMenu.classList.add('open');
  }'''

old_close = '''  function closeMenu() {
    deckMenu.classList.remove('open');
    menuOverlay.style.opacity = '0';
    setTimeout(() => {
      deckMenu.style.display = 'none';
      menuOverlay.style.display = 'none';
    }, 250);
  }'''

new_close = '''  function closeMenu() {
    deckMenu.classList.remove('open');
    deckMenu.setAttribute("style", "display: none !important;");
    menuOverlay.setAttribute("style", "display: none !important;");
  }'''

old_click = '''  counter.addEventListener('click', (e) => {
    e.stopPropagation();
    if (deckMenu.style.display === 'flex') {
      closeMenu();
    } else {
      openMenu();
    }
  });'''

new_click = '''  counter.addEventListener('click', (e) => {
    e.stopPropagation();
    // 采用更稳妥的状态检测
    if (deckMenu.style.display.includes('flex') || deckMenu.classList.contains('open')) {
      closeMenu();
    } else {
      openMenu();
    }
  });'''

c = c.replace(old_open, new_open)
c = c.replace(old_close, new_close)
c = c.replace(old_click, new_click)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Forced foolproof menu display")

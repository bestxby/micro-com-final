# -*- coding: utf-8 -*-
import glob, re

path = 'E:/学习/大二下课程/微机原理与嵌入式系统实验/06_期末大作业_FinalProject/演示文档/slides/section0-02-agenda.html'

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Add hover styles to agenda-card
c = c.replace('.agenda-card {', '.agenda-card {\n    cursor: pointer;\n    transition: all 0.2s ease;')
if '.agenda-card:hover {' not in c:
    c = c.replace('</style>', '  .agenda-card:hover {\n    transform: translateY(-4px);\n    box-shadow: 0 15px 30px -10px rgba(15, 118, 110, 0.15);\n    border-color: rgba(15, 118, 110, 0.3);\n  }\n</style>')

# Add onclick attributes
replacements = [
    ('<div class="glass-card agenda-card">\n      <div class="agenda-num">01</div>', 
     '<div class="glass-card agenda-card" onclick="if(window.parent) window.parent.location.hash=\'#3\'">\n      <div class="agenda-num">01</div>'),
    ('<div class="glass-card agenda-card">\n      <div class="agenda-num">02</div>', 
     '<div class="glass-card agenda-card" onclick="if(window.parent) window.parent.location.hash=\'#5\'">\n      <div class="agenda-num">02</div>'),
    ('<div class="glass-card agenda-card">\n      <div class="agenda-num">03</div>', 
     '<div class="glass-card agenda-card" onclick="if(window.parent) window.parent.location.hash=\'#8\'">\n      <div class="agenda-num">03</div>'),
    ('<div class="glass-card agenda-card">\n      <div class="agenda-num">04</div>', 
     '<div class="glass-card agenda-card" onclick="if(window.parent) window.parent.location.hash=\'#11\'">\n      <div class="agenda-num">04</div>'),
    ('<div class="glass-card agenda-card">\n      <div class="agenda-num">05</div>', 
     '<div class="glass-card agenda-card" onclick="if(window.parent) window.parent.location.hash=\'#15\'">\n      <div class="agenda-num">05</div>'),
    ('<div class="glass-card agenda-card">\n      <div class="agenda-num">06</div>', 
     '<div class="glass-card agenda-card" onclick="if(window.parent) window.parent.location.hash=\'#18\'">\n      <div class="agenda-num">06</div>'),
    ('<div class="glass-card agenda-card">\n      <div class="agenda-num">07</div>', 
     '<div class="glass-card agenda-card" onclick="if(window.parent) window.parent.location.hash=\'#21\'">\n      <div class="agenda-num">07</div>')
]

for old_str, new_str in replacements:
    c = c.replace(old_str, new_str)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(c)

print("SUCCESS: Added click navigation to agenda cards")

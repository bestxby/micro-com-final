# -*- coding: utf-8 -*-
import glob

# Fix section4
path4 = glob.glob('*文档/slides/section4-01-divider-ui.html')[0]
c4 = open(path4, encoding='utf-8').read()
c4 = c4.replace('<div class="topic-item active-topic">\n          <div class="topic-num">05</div>', '<div class="topic-item dim-topic">\n          <div class="topic-num">05</div>')
c4 = c4.replace('<div class="topic-item dim-topic">\n          <div class="topic-num">04</div>', '<div class="topic-item active-topic">\n          <div class="topic-num">04</div>')
open(path4, 'w', encoding='utf-8', newline='\n').write(c4)

# Fix section5
path5 = glob.glob('*文档/slides/section5-01-divider-algorithm.html')[0]
c5 = open(path5, encoding='utf-8').read()
c5 = c5.replace('<div class="topic-item active-topic">\n          <div class="topic-num">04</div>', '<div class="topic-item dim-topic">\n          <div class="topic-num">04</div>')
c5 = c5.replace('<div class="topic-item dim-topic">\n          <div class="topic-num">05</div>', '<div class="topic-item active-topic">\n          <div class="topic-num">05</div>')
open(path5, 'w', encoding='utf-8', newline='\n').write(c5)

print("Swapped 04 and 05 successfully.")

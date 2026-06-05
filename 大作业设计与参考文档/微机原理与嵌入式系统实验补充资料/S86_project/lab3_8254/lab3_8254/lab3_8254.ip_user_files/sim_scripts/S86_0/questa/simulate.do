onbreak {quit -f}
onerror {quit -f}

vsim -t 1ps -lib xil_defaultlib S86_0_opt

do {wave.do}

view wave
view structure
view signals

do {S86_0.udo}

run -all

quit -force

onbreak {quit -f}
onerror {quit -f}

vsim -t 1ps -lib xil_defaultlib Decoder_0_opt

do {wave.do}

view wave
view structure
view signals

do {Decoder_0.udo}

run -all

quit -force

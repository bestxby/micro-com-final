# ============================================================
# JTAG Deploy Script for EES351 Zynq Project
# Usage: xsct jtag_deploy.tcl
# ============================================================

set WORKSPACE "E:/VitisWorks"
set HW_SERVER  "127.0.0.1:3121"

# ---------- 文件路径 ----------
set BITSTREAM   "$WORKSPACE/app_component/_ide/bitstream/EES351_TOP.bit"
set FSBL_ELF    "$WORKSPACE/sdcard/export/sdcard/sw/boot/fsbl.elf"
set APP_ELF     "$WORKSPACE/app_component/build/app_component.elf"
set PS7_TCL     "$WORKSPACE/app_component/_ide/psinit/ps7_init.tcl"

puts "============================================"
puts "  EES351 Zynq JTAG Deploy"
puts "============================================"

# ---------- 1. 连接 Hardware Server ----------
puts "\n[1/5] Connecting to hardware server ($HW_SERVER)..."
connect -url "tcp:$HW_SERVER"

# ---------- 2. 初始化 PS ----------
puts "\n[2/5] Running PS7 initialization..."
source $PS7_TCL
ps7_init
ps7_post_config

# ---------- 3. 下载 FPGA 位流 ----------
puts "\n[3/5] Programming FPGA with bitstream..."
fpga $BITSTREAM
puts "    Bitstream loaded successfully."

# ---------- 4. 下载 FSBL 到 OCM (可选, 但推荐) ----------
puts "\n[4/5] Running FSBL to initialize DDR/SDIO/MIO..."
targets -set -filter {name =~ "ARM*#0"}
# PS7 init already did most of what FSBL does,
# but loading and running FSBL ensures full initialization
dow $FSBL_ELF
# Run FSBL until it reaches handoff point
con -block -timeout 30
puts "    FSBL initialization complete."

# ---------- 5. 下载应用程序 ELF ----------
puts "\n[5/5] Downloading application..."
# Re-target the ARM core after FSBL
targets -set -filter {name =~ "ARM*#0"}
rst -processor
# 重新做一次 ps7_init (因为 FSBL 复位了)
ps7_init
ps7_post_config
dow $APP_ELF

puts "============================================"
puts "  Deploy complete! Starting application..."
puts "  The program will now run on the ARM core."
puts "  Check the SD card for log.txt after running."
puts "============================================"

# ---------- 启动运行 ----------
con

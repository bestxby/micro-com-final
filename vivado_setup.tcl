# vivado_setup.tcl
# 这是一个为 Vivado 准备的自动化 Tcl 脚本。
# 它将自动创建 Block Design，配置 Zynq7 开启 SD 卡与 UART1 (EMIO)，并自动引出接口。

puts "========================================================="
puts "开始执行 Zynq PS 配置脚本 (开启 SD 卡与 EMIO UART1)..."
puts "========================================================="

# 1. 尝试打开工程 (如果当前未打开)
set proj_path "e:/Micro_Project/vivado/LCD_Project/LCD_Project.xpr"
if {[current_project -quiet] eq ""} {
    if {[file exists $proj_path]} {
        open_project $proj_path
    } else {
        puts "ERROR: 找不到工程 $proj_path"
        return
    }
}

# 2. 创建或打开 Block Design
set bd_name "system"
if {[get_bd_designs -quiet $bd_name] eq ""} {
    create_bd_design $bd_name
} else {
    open_bd_design [get_bd_designs $bd_name]
}

# 3. 实例化 Zynq PS 核心
set ps7_name "processing_system7_0"
if {[get_bd_cells -quiet $ps7_name] eq ""} {
    create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:* $ps7_name
    apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  [get_bd_cells $ps7_name]
}

# 4. 配置 Zynq 开启 SD0 (MIO) 与 UART1 (EMIO)
set_property -dict [list \
  CONFIG.PCW_SD0_PERIPHERAL_ENABLE {1} \
  CONFIG.PCW_SD0_SD0_IO {MIO 40 .. 45} \
  CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} \
  CONFIG.PCW_UART1_UART1_IO {EMIO} \
] [get_bd_cells $ps7_name]

# 5. 引出 EMIO UART 接口
set uart_intf [get_bd_intf_pins $ps7_name/UART_1]
# 如果尚未引出，则将其设为 External
if {[get_bd_intf_ports -quiet UART_1_0] eq ""} {
    make_bd_intf_pins_external  $uart_intf
}

# 6. 保存并验证 Block Design
save_bd_design
validate_bd_design

# 7. 生成 Wrapper (顶层模块)
set bd_file [get_files ${bd_name}.bd]
set wrapper_path [make_wrapper -files $bd_file -top]
add_files -norecurse $wrapper_path
# 确保 EES351_TOP 依然是顶层模块
set_property top EES351_TOP [current_fileset]
update_compile_order -fileset sources_1

puts "========================================================="
puts "配置完成！由于您已经让我帮您改好了 EES351_TOP.v 和 XDC，"
puts "您现在直接点击左侧的 Generate Bitstream 即可！"
puts "========================================================="

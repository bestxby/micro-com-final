# fix_vivado.tcl
puts "============================================="
puts "正在修复 Vivado 综合报错与连线缺失问题..."
puts "============================================="

# 1. 修复 AXI 时钟悬空报错
catch {
    connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK]
}
save_bd_design

# 2. 强制为 system.bd 生成 Output Products 和 Wrapper
set bd_file [get_files system.bd]
generate_target all $bd_file
set wrapper_path [make_wrapper -files $bd_file -top]
add_files -norecurse -force $wrapper_path

# 3. 修复黄色的 LCD.v 路径丢失警告
set missing_lcd "E:/Micro_Project/vivado/LCD_Project/LCD_Project.srcs/sources_1/new/LCD.v"
if {[get_files -quiet $missing_lcd] ne ""} {
    remove_files $missing_lcd
}
catch {
    add_files -norecurse "E:/Micro_Project/vivado/source/sources_1/new/LCD.v"
}

# 4. 重新指定正确的顶层模块
set_property top EES351_TOP [current_fileset]
update_compile_order -fileset sources_1

puts "============================================="
puts "修复全部完成！请直接再次点击左侧 Generate Bitstream。"
puts "============================================="

transcript off
onbreak {quit -force}
onerror {quit -force}
transcript on

vlib work
vlib riviera/xilinx_vip
vlib riviera/axi_infrastructure_v1_1_0
vlib riviera/axi_vip_v1_1_22
vlib riviera/processing_system7_vip_v1_0_24
vlib riviera/xil_defaultlib

vmap xilinx_vip riviera/xilinx_vip
vmap axi_infrastructure_v1_1_0 riviera/axi_infrastructure_v1_1_0
vmap axi_vip_v1_1_22 riviera/axi_vip_v1_1_22
vmap processing_system7_vip_v1_0_24 riviera/processing_system7_vip_v1_0_24
vmap xil_defaultlib riviera/xil_defaultlib

vlog -work xilinx_vip  -incr "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" -l xilinx_vip -l axi_infrastructure_v1_1_0 -l axi_vip_v1_1_22 -l processing_system7_vip_v1_0_24 -l xil_defaultlib \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi4stream_vip_axi4streampc.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi_vip_axi4pc.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/xil_common_vip_pkg.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi4stream_vip_pkg.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi_vip_pkg.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi4stream_vip_if.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi_vip_if.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/clk_vip_if.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/rst_vip_if.sv" \

vlog -work axi_infrastructure_v1_1_0  -incr -v2k5 "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" -l xilinx_vip -l axi_infrastructure_v1_1_0 -l axi_vip_v1_1_22 -l processing_system7_vip_v1_0_24 -l xil_defaultlib \
"../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl/axi_infrastructure_v1_1_vl_rfs.v" \

vlog -work axi_vip_v1_1_22  -incr "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" -l xilinx_vip -l axi_infrastructure_v1_1_0 -l axi_vip_v1_1_22 -l processing_system7_vip_v1_0_24 -l xil_defaultlib \
"../../../../LCD_Project.gen/sources_1/bd/system/ipshared/b16a/hdl/axi_vip_v1_1_vl_rfs.sv" \

vlog -work processing_system7_vip_v1_0_24  -incr "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" -l xilinx_vip -l axi_infrastructure_v1_1_0 -l axi_vip_v1_1_22 -l processing_system7_vip_v1_0_24 -l xil_defaultlib \
"../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl/processing_system7_vip_v1_0_vl_rfs.sv" \

vlog -work xil_defaultlib  -incr -v2k5 "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" -l xilinx_vip -l axi_infrastructure_v1_1_0 -l axi_vip_v1_1_22 -l processing_system7_vip_v1_0_24 -l xil_defaultlib \
"../../../bd/system/ip/system_processing_system7_0_0/sim/system_processing_system7_0_0.v" \
"../../../bd/system/sim/system.v" \

vlog -work xil_defaultlib \
"glbl.v"


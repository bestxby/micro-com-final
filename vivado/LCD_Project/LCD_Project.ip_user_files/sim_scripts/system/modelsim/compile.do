vlib modelsim_lib/work
vlib modelsim_lib/msim

vlib modelsim_lib/msim/xilinx_vip
vlib modelsim_lib/msim/axi_infrastructure_v1_1_0
vlib modelsim_lib/msim/axi_vip_v1_1_22
vlib modelsim_lib/msim/processing_system7_vip_v1_0_24
vlib modelsim_lib/msim/xil_defaultlib

vmap xilinx_vip modelsim_lib/msim/xilinx_vip
vmap axi_infrastructure_v1_1_0 modelsim_lib/msim/axi_infrastructure_v1_1_0
vmap axi_vip_v1_1_22 modelsim_lib/msim/axi_vip_v1_1_22
vmap processing_system7_vip_v1_0_24 modelsim_lib/msim/processing_system7_vip_v1_0_24
vmap xil_defaultlib modelsim_lib/msim/xil_defaultlib

vlog -work xilinx_vip  -incr -mfcu  -sv -L axi_vip_v1_1_22 -L processing_system7_vip_v1_0_24 -L xilinx_vip "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi4stream_vip_axi4streampc.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi_vip_axi4pc.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/xil_common_vip_pkg.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi4stream_vip_pkg.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi_vip_pkg.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi4stream_vip_if.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/axi_vip_if.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/clk_vip_if.sv" \
"D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/hdl/rst_vip_if.sv" \

vlog -work axi_infrastructure_v1_1_0  -incr -mfcu  "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" \
"../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl/axi_infrastructure_v1_1_vl_rfs.v" \

vlog -work axi_vip_v1_1_22  -incr -mfcu  -sv -L axi_vip_v1_1_22 -L processing_system7_vip_v1_0_24 -L xilinx_vip "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" \
"../../../../LCD_Project.gen/sources_1/bd/system/ipshared/b16a/hdl/axi_vip_v1_1_vl_rfs.sv" \

vlog -work processing_system7_vip_v1_0_24  -incr -mfcu  -sv -L axi_vip_v1_1_22 -L processing_system7_vip_v1_0_24 -L xilinx_vip "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" \
"../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl/processing_system7_vip_v1_0_vl_rfs.sv" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/ec67/hdl" "+incdir+../../../../LCD_Project.gen/sources_1/bd/system/ipshared/9a25/hdl" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/rsb/busdef" "+incdir+D:/Vivado_zync_2025/2025.2/Vivado/data/xilinx_vip/include" \
"../../../bd/system/ip/system_processing_system7_0_0/sim/system_processing_system7_0_0.v" \
"../../../bd/system/sim/system.v" \

vlog -work xil_defaultlib \
"glbl.v"


set_property SRC_FILE_INFO {cfile:{c:/Users/Administrator/Desktop/lab3_8254 2/lab3_8254/lab3_8254.srcs/sources_1/ip/pll/pll.xdc} rfile:../lab3_8254.srcs/sources_1/ip/pll/pll.xdc id:1 order:EARLY scoped_inst:CLOCK/inst} [current_design]
set_property SRC_FILE_INFO {cfile:{C:/Users/Administrator/Desktop/lab3_8254 2/lab3_8254/lab3_8254.srcs/constrs_1/new/constraints.xdc} rfile:../lab3_8254.srcs/constrs_1/new/constraints.xdc id:2} [current_design]
current_instance CLOCK/inst
set_property src_info {type:SCOPED_XDC file:1 line:57 export:INPUT save:INPUT read:READ} [current_design]
set_input_jitter [get_clocks -of_objects [get_ports clk_in1]] 0.1
current_instance
set_property src_info {type:XDC file:2 line:27 export:INPUT save:INPUT read:READ} [current_design]
set_property IOSTANDARD LVCMOS33 [get_ports led_]
set_property src_info {type:XDC file:2 line:48 export:INPUT save:INPUT read:READ} [current_design]
set_property PACKAGE_PIN A21 [get_ports led_]
set_property src_info {type:XDC file:2 line:56 export:INPUT save:INPUT read:READ} [current_design]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets btn_IBUF]
set_property src_info {type:XDC file:2 line:78 export:INPUT save:INPUT read:READ} [current_design]
set_property IOSTANDARD LVCMOS33 [get_ports clk_1HZ]
set_property src_info {type:XDC file:2 line:79 export:INPUT save:INPUT read:READ} [current_design]
set_property PACKAGE_PIN G20 [get_ports clk_1HZ]

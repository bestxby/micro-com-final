// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Mon Aug  5 09:45:57 2024
// Host        : LAPTOP-42E9R2TH running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               e:/EES-351-V1.1/project/lab1_io/lab1_io.srcs/sources_1/ip/S86_0/S86_0_stub.v
// Design      : S86_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg484-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "zet,Vivado 2019.1" *)
module S86_0(wb_clk_i, wb_rst_i, wb_dat_i, wb_dat_o, wb_adr_o, 
  wb_we_o, wb_tga_o, wb_sel_o, wb_stb_o, wb_cyc_o, wb_ack_i, wb_tgc_i, wb_tgc_o, nmi, nmia, pc)
/* synthesis syn_black_box black_box_pad_pin="wb_clk_i,wb_rst_i,wb_dat_i[15:0],wb_dat_o[15:0],wb_adr_o[19:1],wb_we_o,wb_tga_o,wb_sel_o[1:0],wb_stb_o,wb_cyc_o,wb_ack_i,wb_tgc_i,wb_tgc_o,nmi,nmia,pc[19:0]" */;
  input wb_clk_i;
  input wb_rst_i;
  input [15:0]wb_dat_i;
  output [15:0]wb_dat_o;
  output [19:1]wb_adr_o;
  output wb_we_o;
  output wb_tga_o;
  output [1:0]wb_sel_o;
  output wb_stb_o;
  output wb_cyc_o;
  input wb_ack_i;
  input wb_tgc_i;
  output wb_tgc_o;
  input nmi;
  output nmia;
  output [19:0]pc;
endmodule

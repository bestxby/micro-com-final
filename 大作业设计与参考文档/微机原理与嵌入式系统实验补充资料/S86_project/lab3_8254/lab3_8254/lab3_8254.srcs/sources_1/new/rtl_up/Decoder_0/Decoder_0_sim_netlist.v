// Copyright 1986-2015 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2015.4 (win64) Build 1412921 Wed Nov 18 09:43:45 MST 2015
// Date        : Sat Aug 19 15:30:23 2017
// Host        : hp-PC running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode funcsim
//               c:/Users/hp/Desktop/sysclassfiles/interface/solution/Ex_5/Ex_5_S8254/Ex_5_S8254.srcs/sources_1/ip/Decoder_0/Decoder_0_sim_netlist.v
// Design      : Decoder_0
// Purpose     : This verilog netlist is a functional simulation representation of the design and should not be modified
//               or synthesized. This netlist cannot be used for SDF annotated simulation.
// Device      : xc7a100tfgg484-1
// --------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* CHECK_LICENSE_TYPE = "Decoder_0,Decoder,{}" *) (* CORE_GENERATION_INFO = "Decoder_0,Decoder,{x_ipProduct=Vivado 2015.4,x_ipVendor=xilinx.com,x_ipLibrary=user,x_ipName=Decoder,x_ipVersion=1.0,x_ipCoreRevision=2,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED}" *) (* DowngradeIPIdentifiedWarnings = "yes" *) 
(* X_CORE_INFO = "Decoder,Vivado 2015.4" *) 
(* NotValidForBitStream *)
module Decoder_0
   (A,
    B,
    C,
    G1,
    G2AN,
    G2BN,
    Y0N,
    Y1N,
    Y2N,
    Y3N,
    Y4N,
    Y5N,
    Y6N,
    Y7N);
  input A;
  input B;
  input C;
  input G1;
  input G2AN;
  input G2BN;
  output Y0N;
  output Y1N;
  output Y2N;
  output Y3N;
  output Y4N;
  output Y5N;
  output Y6N;
  output Y7N;

  wire A;
  wire B;
  wire C;
  wire G1;
  wire G2AN;
  wire G2BN;
  wire Y0N;
  wire Y1N;
  wire Y2N;
  wire Y3N;
  wire Y4N;
  wire Y5N;
  wire Y6N;
  wire Y7N;

  (* black_box = "1" *) 
  Decoder_0_Decoder inst
       (.A(A),
        .B(B),
        .C(C),
        .G1(G1),
        .G2AN(G2AN),
        .G2BN(G2BN),
        .Y0N(Y0N),
        .Y1N(Y1N),
        .Y2N(Y2N),
        .Y3N(Y3N),
        .Y4N(Y4N),
        .Y5N(Y5N),
        .Y6N(Y6N),
        .Y7N(Y7N));
endmodule

(* ORIG_REF_NAME = "Decoder" *) 
module Decoder_0_Decoder
   (A,
    B,
    C,
    G1,
    G2AN,
    G2BN,
    Y0N,
    Y1N,
    Y2N,
    Y3N,
    Y4N,
    Y5N,
    Y6N,
    Y7N);
  input A;
  input B;
  input C;
  input G1;
  input G2AN;
  input G2BN;
  output Y0N;
  output Y1N;
  output Y2N;
  output Y3N;
  output Y4N;
  output Y5N;
  output Y6N;
  output Y7N;

  wire A;
  wire B;
  wire C;
  wire G1;
  wire G2AN;
  wire G2BN;
  wire Y0N;
  wire Y1N;
  wire Y2N;
  wire Y3N;
  wire Y4N;
  wire Y5N;
  wire Y6N;
  wire Y7N;

  LUT6 #(
    .INIT(64'hFFFFFFFFFFFFFFFB)) 
    Y0N_INST_0
       (.I0(G2AN),
        .I1(G1),
        .I2(G2BN),
        .I3(B),
        .I4(A),
        .I5(C),
        .O(Y0N));
  LUT6 #(
    .INIT(64'hFFFFFFFFFFFFFFBF)) 
    Y1N_INST_0
       (.I0(G2BN),
        .I1(G1),
        .I2(A),
        .I3(B),
        .I4(G2AN),
        .I5(C),
        .O(Y1N));
  LUT6 #(
    .INIT(64'hFFFFFFFFFFFFFFBF)) 
    Y2N_INST_0
       (.I0(G2BN),
        .I1(G1),
        .I2(B),
        .I3(C),
        .I4(G2AN),
        .I5(A),
        .O(Y2N));
  LUT6 #(
    .INIT(64'hFFFFFFFFFFFFFF7F)) 
    Y3N_INST_0
       (.I0(G1),
        .I1(A),
        .I2(B),
        .I3(C),
        .I4(G2BN),
        .I5(G2AN),
        .O(Y3N));
  LUT6 #(
    .INIT(64'hFFFFFFFFFFFFFFBF)) 
    Y4N_INST_0
       (.I0(G2BN),
        .I1(G1),
        .I2(C),
        .I3(B),
        .I4(G2AN),
        .I5(A),
        .O(Y4N));
  LUT6 #(
    .INIT(64'hFFFFFFFFFFFFFF7F)) 
    Y5N_INST_0
       (.I0(G1),
        .I1(A),
        .I2(C),
        .I3(B),
        .I4(G2BN),
        .I5(G2AN),
        .O(Y5N));
  LUT6 #(
    .INIT(64'hFFFFFFFFFFFFFF7F)) 
    Y6N_INST_0
       (.I0(G1),
        .I1(C),
        .I2(B),
        .I3(A),
        .I4(G2BN),
        .I5(G2AN),
        .O(Y6N));
  LUT6 #(
    .INIT(64'hFFFFFFFFFF7FFFFF)) 
    Y7N_INST_0
       (.I0(G1),
        .I1(A),
        .I2(C),
        .I3(G2AN),
        .I4(B),
        .I5(G2BN),
        .O(Y7N));
endmodule
`ifndef GLBL
`define GLBL
`timescale  1 ps / 1 ps

module glbl ();

    parameter ROC_WIDTH = 100000;
    parameter TOC_WIDTH = 0;

//--------   STARTUP Globals --------------
    wire GSR;
    wire GTS;
    wire GWE;
    wire PRLD;
    tri1 p_up_tmp;
    tri (weak1, strong0) PLL_LOCKG = p_up_tmp;

    wire PROGB_GLBL;
    wire CCLKO_GLBL;
    wire FCSBO_GLBL;
    wire [3:0] DO_GLBL;
    wire [3:0] DI_GLBL;
   
    reg GSR_int;
    reg GTS_int;
    reg PRLD_int;

//--------   JTAG Globals --------------
    wire JTAG_TDO_GLBL;
    wire JTAG_TCK_GLBL;
    wire JTAG_TDI_GLBL;
    wire JTAG_TMS_GLBL;
    wire JTAG_TRST_GLBL;

    reg JTAG_CAPTURE_GLBL;
    reg JTAG_RESET_GLBL;
    reg JTAG_SHIFT_GLBL;
    reg JTAG_UPDATE_GLBL;
    reg JTAG_RUNTEST_GLBL;

    reg JTAG_SEL1_GLBL = 0;
    reg JTAG_SEL2_GLBL = 0 ;
    reg JTAG_SEL3_GLBL = 0;
    reg JTAG_SEL4_GLBL = 0;

    reg JTAG_USER_TDO1_GLBL = 1'bz;
    reg JTAG_USER_TDO2_GLBL = 1'bz;
    reg JTAG_USER_TDO3_GLBL = 1'bz;
    reg JTAG_USER_TDO4_GLBL = 1'bz;

    assign (weak1, weak0) GSR = GSR_int;
    assign (weak1, weak0) GTS = GTS_int;
    assign (weak1, weak0) PRLD = PRLD_int;

    initial begin
	GSR_int = 1'b1;
	PRLD_int = 1'b1;
	#(ROC_WIDTH)
	GSR_int = 1'b0;
	PRLD_int = 1'b0;
    end

    initial begin
	GTS_int = 1'b1;
	#(TOC_WIDTH)
	GTS_int = 1'b0;
    end

endmodule
`endif

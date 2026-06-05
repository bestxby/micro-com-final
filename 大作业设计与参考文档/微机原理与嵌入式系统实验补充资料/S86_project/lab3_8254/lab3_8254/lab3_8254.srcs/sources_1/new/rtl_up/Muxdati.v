`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
module Muxdati(
    input wb_tga_i,
    input RDN,
    input [15:0] MEMDATI,
    input IOCS0_N,
    input [15:0] IODAT0I,
    input IOCS1_N,
    input [15:0] IODAT1I,
    output [15:0] DATO
    );
    
    assign DATO =  RDN ? 1'b0 : 
                  ((!wb_tga_i) ? MEMDATI :
                  ((!IOCS0_N)  ? IODAT0I :
                  ((!IOCS1_N)  ? IODAT1I : 0))); 
               
endmodule

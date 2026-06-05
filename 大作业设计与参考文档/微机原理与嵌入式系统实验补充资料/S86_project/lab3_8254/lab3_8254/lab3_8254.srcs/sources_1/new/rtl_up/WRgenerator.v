`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
module WRgenerator(
    input wb_tga_i,
    input wb_we_i,
    input wb_cyc_i,
    input wb_stb_i,
    output IOR_N,
    output IOW_N,
    output MEMW_N,
    output MEMR_N
    );
    
    assign IOR_N =  ~((wb_tga_i) & (!wb_we_i) & (wb_cyc_i) & (wb_stb_i));
    assign IOW_N =  ~((wb_tga_i) & (wb_we_i) & (wb_cyc_i) & (wb_stb_i));
    assign MEMR_N = ~((!wb_tga_i) & (!wb_we_i) & (wb_cyc_i) & (wb_stb_i));
    assign MEMW_N = ~((!wb_tga_i) & (wb_we_i) & (wb_cyc_i) & (wb_stb_i));
    
endmodule

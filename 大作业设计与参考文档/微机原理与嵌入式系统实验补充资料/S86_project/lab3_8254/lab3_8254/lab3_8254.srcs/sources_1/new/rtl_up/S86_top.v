`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
module S86_top (
    input         clk_100_,
    input         button_rst,
    input           btn,
    output        out0,
    //switch
    input  [11:0] switch_,
    //segs
    output [7:0] an,
    output [7:0] segs1,
    output [7:0] segs2
 );
    wire    sw12;
    //clock
    wire  clk, clk_ram, lock;
    //reset
    wire  rst;
    //WB总线
    wire [15:0] dat_o;
    wire [15:0] dat_i;
    wire [19:1] adr;
    wire [1:0]  sel;
    wire we, tga, stb, cyc, ack;
    //读写发生器:ior/iow/memw/memr     
    wire ior_n, iow_n, memw_n, memr_n; 
    //地址译码器
    wire swcsn, digitcsn, s8254csn; 
    //RAM
    wire [15:0] mem_dat_i;
    wire        memack;
    //switch
    wire [15:0] switch_out;
    //S8254
    wire [7:0] s8254out;
 
    pll CLOCK(.clk_in1 (clk_100_), .clk_out1(clk),.clk_out2(clk_ram),.locked (lock));
    
    reset RESET(.lock(lock),.button(~button_rst),.reset(rst));
 
    //S86 processor
    S86_0 S86_proc (
        //Wishbone master interface
        .wb_clk_i (clk),
        .wb_rst_i (rst),
        .wb_dat_i (dat_i),
        .wb_dat_o (dat_o),
        .wb_adr_o (adr),
        .wb_we_o  (we),
        .wb_tga_o (tga),
        .wb_sel_o (sel),
        .wb_stb_o (stb),
        .wb_cyc_o (cyc),
        .wb_ack_i (ack),
        .wb_tgc_i (1'b0),
        .nmi(1'b0)
    );
         
    WRgenerator U0 (
        .wb_tga_i(tga),
        .wb_we_i(we),
        .wb_cyc_i(cyc),
        .wb_stb_i(stb),
        .IOR_N(ior_n),
        .IOW_N(iow_n),
        .MEMW_N(memw_n),
        .MEMR_N(memr_n)
    );  
 
    Decoder_0 U1(
        .A(adr[4]),                                                   
        .B(adr[5]),                                                  
        .C(adr[6]),                                                  
        .G1(!adr[7]),                                                
        .G2AN(adr[9]|adr[8]),                                         
        .G2BN(ior_n & iow_n),                                                 
        .Y1N(swcsn),
        .Y2N(digitcsn),
        .Y4N(s8254csn)                                                  
    );
    
    //输入数据多路选择器
    Muxdati U2(
        .wb_tga_i(tga),
        .RDN(ior_n & memr_n),
        .MEMDATI(mem_dat_i),
        .IOCS0_N(swcsn),
        .IODAT0I(switch_out),
        .IOCS1_N(s8254csn),
        .IODAT1I({8'h00,s8254out}),
        .DATO(dat_i)
    );
    
    //应答信号发生器
    ACKgenerator U3( 
        .MEMACK(memack),
        .IOCS0_N(swcsn),
        .IOCS1_N(digitcsn),
        .IOCS2_N(s8254csn),
        .ACK(ack)
    );
    
    RAM_BIOS M0(
        .wb_clk_i (clk_ram),
        .wb_dat_o (mem_dat_i),
        .wb_dat_i (dat_o),
        .wb_adr_i (adr),
        .wb_sel_i (sel),
        .wb_ack_o (memack),   
        .MEMW_N (memw_n),
        .MEMR_N (memr_n)
    );

    Switch D0(
         .wb_dat_o(switch_out),
         .wb_adr_i(adr[1]),
         .IOR_N(ior_n),
         .CS_N(swcsn),
         .switch_i(switch_)
    );  
        
    Digit D1(
        .wb_clk_i(clk),
        .wb_rst_i(rst), 
        .wb_dat_i(dat_o),
        .wb_adr_i(adr[2:1]),
        .IOW_N(iow_n),
        .CS_N(digitcsn),
        .an(an),
        .segs1(segs1),
        .segs2(segs2)
    );
   key_latch   key_latch_0(
    .key_in     (btn),
    .key_out    (sw12)
    ); 
wire clk_1HZ;//1Mhz 
    divclk U4(.clk(clk), .rst(rst),.clk_sys(clk_1HZ)); 
    
    S_8254 D2(    
        .clk0(clk_1HZ),      // Counter0输入时钟
        .gate0(1'b1), // Counter0使能
        .out0(out0),     // Counter0输出波形
        .a(adr[2:1]),        // 地址线
        .id(dat_o[7:0]),     // 输入数据
        .od(s8254out), // 输出数据
        .CS_N(s8254csn),     // 片选
        .IOR_N(ior_n),       // 读信号
        .IOW_N(iow_n)        // 写信号
    );  
    
endmodule

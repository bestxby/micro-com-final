`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// 模块功能：基于Wishbone总线的8位数码管动态扫描驱动（16位总线→8个不同数字）
// 端口说明：
// - wb_clk_i: Wishbone总线时钟
// - wb_rst_i: 复位信号（高有效）
// - wb_adr_i: 地址总线（2位）：00=写低16位数据(对应数码管DK3-DK0)，01=写高16位数据(对应数码管DK7-DK4)，10=写控制寄存器
// - wb_dat_i: 16位数据总线，分两次写入32位显示数据
// - IOW_N: I/O写信号（低有效）
// - CS_N: 片选信号（低有效）
// - an: 8位数码管选通信号（高电平选通）
// - segs: 8位段码输出（共阴极，bit0=DP, bit1=A, ..., bit7=G）
//////////////////////////////////////////////////////////////////////////////////

module Digit(
    // wishbone interface
    input        wb_clk_i,    // Clock
    input        wb_rst_i,    // Reset
    input [1:0]  wb_adr_i,    // address bus inputs
    input [15:0] wb_dat_i,    // 16位输入数据总线
    input        IOW_N,
    input        CS_N,
    output reg [7:0] an,      //八个数码管的选通信号，高电平选通
    output [7:0] segs1,//右边4位数码管的8段点亮信号 
    output [7:0] segs2//左边4位数码管的8段点亮信号 
);
    
    // -------------------------- 参数定义 --------------------------
    // 分频参数：10MHz时钟分频到1ms（周期1ms，扫描频率1kHz）
    parameter DIV_THRESHOLD = 10000;  
    // 数码管选通状态参数（语义化命名）
    parameter [2:0] DIGIT_DK7 = 3'b000,  // 左数第1个数码管（an[7]）
                    DIGIT_DK6 = 3'b001,  // 左数第2个数码管（an[6]）
                    DIGIT_DK5 = 3'b010,  // 左数第3个数码管（an[5]）
                    DIGIT_DK4 = 3'b011,  // 左数第4个数码管（an[4]）
                    DIGIT_DK3 = 3'b100,  // 左数第5个数码管（an[3]）
                    DIGIT_DK2 = 3'b101,  // 左数第6个数码管（an[2]）
                    DIGIT_DK1 = 3'b110,  // 左数第7个数码管（an[1]）
                    DIGIT_DK0 = 3'b111;  // 左数第8个数码管（an[0]）
  
    // -------------------------- 内部寄存器定义 --------------------------
    reg clk_sys;            // 分频后时钟（1kHz，周期1ms）
    reg [16:0] div_counter; // 分频计数器
    reg [31:0] Ldata_32;    // 32位显示数据寄存器
    reg [7:0] ctrl;         // 8位控制寄存器（每bit对应一个数码管使能）
    reg [3:0] value;        // 当前显示的4位十六进制值
    reg [7:0] segout;       // 段码输出寄存器
    reg [2:0] state;        // 数码管扫描状态寄存器
    reg [7:0] SEG_TABLE[0:15]; // 定义reg型数组（变量），可赋值
 // 共阴极数码管段码表（bit0=DP, bit1=A, bit2=B, bit3=C, bit4=D, bit5=E, bit6=F, bit7=G）
// 初始化段码表（仅仿真/综合时执行一次）
initial begin
    SEG_TABLE[0]  = 8'h3f; // 0
    SEG_TABLE[1]  = 8'h06; // 1
    SEG_TABLE[2]  = 8'h5b; // 2
    SEG_TABLE[3]  = 8'h4f; // 3
    SEG_TABLE[4]  = 8'h66; // 4
    SEG_TABLE[5]  = 8'h6d; // 5
    SEG_TABLE[6]  = 8'h7d; // 6
    SEG_TABLE[7]  = 8'h07; // 7
    SEG_TABLE[8]  = 8'h7f; // 8
    SEG_TABLE[9]  = 8'h6f; // 9
    SEG_TABLE[10] = 8'h77; // A
    SEG_TABLE[11] = 8'h7c; // B
    SEG_TABLE[12] = 8'h39; // C
    SEG_TABLE[13] = 8'h5e; // D
    SEG_TABLE[14] = 8'h79; // E
    SEG_TABLE[15] = 8'h71; // F
end
    // -------------------------- 时钟分频逻辑 --------------------------
    always @(posedge wb_clk_i or posedge wb_rst_i) begin
        if(wb_rst_i) begin
            clk_sys     <= 1'b0;
            div_counter <= 17'd0;
        end else if(div_counter >= DIV_THRESHOLD - 1'b1) begin
            clk_sys     <= ~clk_sys;
            div_counter <= 17'd0;
        end else begin
            div_counter <= div_counter + 1'b1;
        end
    end

    // -------------------------- Wishbone总线写逻辑（16位→32位） --------------------------
    always @(posedge wb_clk_i) begin
        if(wb_rst_i) begin
            Ldata_32 <= 32'h00000000;
            ctrl     <= 8'hff;
        end else begin
            // 地址00：写32位数据的低16位（对应第0-3位数码管：bit15-0）
            if((!CS_N) & (!IOW_N) & (!wb_adr_i[1]) & (!wb_adr_i[0])) begin
                Ldata_32[15:0] <= wb_dat_i;
            end
            // 地址01：写32位数据的高16位（对应第4-7位数码管：bit31-16）
            else if((!CS_N) & (!IOW_N) & (!wb_adr_i[1]) & (wb_adr_i[0])) begin
                Ldata_32[31:16] <= wb_dat_i;
            end
            // 地址10：写控制寄存器（数码管位选通）
            else if((!CS_N) & (!IOW_N) & (wb_adr_i[1]) & (!wb_adr_i[0])) begin
                ctrl <= wb_dat_i[7:0];
            end
            // 地址11：无效，保持原值
            else begin
                Ldata_32 <= Ldata_32;
                ctrl     <= ctrl;
            end
        end
    end

    // -------------------------- 数码管动态扫描逻辑（8个不同数字） --------------------------
    always @(posedge clk_sys or posedge wb_rst_i) begin
        if(wb_rst_i) begin
            state <= DIGIT_DK7;
            an    <= 8'h00;  // 复位时全不选通（低电平）
            value <= 4'h0;
        end else begin
            case(state)
                // 第1个数码管（DK7，an[7]）：显示Ldata_32[31:28]
                DIGIT_DK7: begin
                    an <= ctrl[7] ? 8'b1000_0000 : 8'h00; 
                    value <= Ldata_32[31:28];
                    state <= DIGIT_DK6;
                end
                // 第2个数码管（DK6，an[6]）：显示Ldata_32[27:24]
                DIGIT_DK6: begin
                    an <= ctrl[6] ? 8'b0100_0000 : 8'h00;
                    value <= Ldata_32[27:24];
                    state <= DIGIT_DK5;
                end
                // 第3个数码管（DK5，an[5]）：显示Ldata_32[23:20]
                DIGIT_DK5: begin
                    an <= ctrl[5] ? 8'b0010_0000 : 8'h00;
                    value <= Ldata_32[23:20];
                    state <= DIGIT_DK4;
                end
                // 第4个数码管（DK4，an[4]）：显示Ldata_32[19:16]
                DIGIT_DK4: begin
                    an <= ctrl[4] ? 8'b0001_0000 : 8'h00;
                    value <= Ldata_32[19:16];
                    state <= DIGIT_DK3;
                end
                // 第5个数码管（DK3，an[3]）：显示Ldata_32[15:12]
                DIGIT_DK3: begin
                    an <= ctrl[3] ? 8'b0000_1000 : 8'h00;
                    value <= Ldata_32[15:12];
                    state <= DIGIT_DK2;
                end
                // 第6个数码管（DK2，an[2]）：显示Ldata_32[11:8]
                DIGIT_DK2: begin
                    an <= ctrl[2] ? 8'b0000_0100 : 8'h00;
                    value <= Ldata_32[11:8];
                    state <= DIGIT_DK1;
                end
                // 第7个数码管（DK1，an[1]）：显示Ldata_32[7:4]
                DIGIT_DK1: begin
                    an <= ctrl[1] ? 8'b0000_0010 : 8'h00;
                    value <= Ldata_32[7:4];
                    state <= DIGIT_DK0;
                end
                // 第8个数码管（DK0，an[0]）：显示Ldata_32[3:0]
                DIGIT_DK0: begin
                    an <= ctrl[0] ? 8'b0000_0001 : 8'h00;
                    value <= Ldata_32[3:0];
                    state <= DIGIT_DK7; // 循环扫描
                end
                default: begin
                    an <= 8'h00;
                    value <= 4'h0;
                    state <= DIGIT_DK7;
                end
            endcase
        end
    end

    // -------------------------- 段码译码逻辑 --------------------------
    always @(*) begin
        segout = SEG_TABLE[value]; // 查表得到段码
    end

    // 将segout连接到接口信号segs, 第一组输出segs1，第二组输出segs2
assign segs2 = segout;
assign segs1 = segout;  

endmodule
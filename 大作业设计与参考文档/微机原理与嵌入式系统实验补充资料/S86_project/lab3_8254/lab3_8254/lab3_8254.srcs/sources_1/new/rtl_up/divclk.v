`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////

module divclk(clk,rst,clk_sys);   //分频模块
	input clk,rst;
	output reg clk_sys;
	
	reg [31:0] div_counter;
	always @(posedge clk or posedge rst)     //时钟上升沿
        if(rst) begin
           clk_sys <= 0;
           div_counter <= 0;
        end
      else if(div_counter >= 4) begin  // 用于信号发生器的代码
//       else if(div_counter >= 5000000) begin  用于计数器的代码
			clk_sys <= ~clk_sys;
			div_counter <= 0;
		end
		else begin
			div_counter <= div_counter + 1;
		end
endmodule

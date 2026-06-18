`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/11/20 10:22:11
// Design Name: 
// Module Name: LCD
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module LCD(
    input      LED_ARM     ,
    input      DC_ARM      ,
    input      CS_ARM      ,
    input      RST_ARM     ,
    input      SCK_ARM     ,
    input      SDI_ARM     ,
    
    output       LED_BASE   ,
    output       DC_BASE    ,
    output       CS_BASE    ,
    output       RST_BASE   ,
    output       SCK_BASE   ,
    output       SDI_BASE   
    ); 
assign LED_BASE     = LED_ARM  ; 
assign DC_BASE      = DC_ARM      ; 
assign CS_BASE      = CS_ARM      ; 
assign RST_BASE     = RST_ARM     ; 
assign SCK_BASE     = SCK_ARM     ; 
assign SDI_BASE     = SDI_ARM     ; 
    
endmodule

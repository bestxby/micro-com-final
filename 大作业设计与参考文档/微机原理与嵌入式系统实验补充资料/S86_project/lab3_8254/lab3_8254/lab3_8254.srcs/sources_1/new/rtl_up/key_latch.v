`timescale 1ns/1ps
module key_latch(
    input       wire        key_in,
    output      reg         key_out
    );
always@(posedge key_in )
    if(key_out ==  1'b1)
        key_out <= 1'b0;
    else if(key_out == 1'b0)
        key_out <= 1'b1;
endmodule

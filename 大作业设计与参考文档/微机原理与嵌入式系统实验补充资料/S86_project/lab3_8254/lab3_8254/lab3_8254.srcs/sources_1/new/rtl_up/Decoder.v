`timescale 1ns / 1ps

module Decoder_0(
    input A,
    input B,
    input C,
    input G1,
    input G2AN,
    input G2BN,
    output Y0N,
    output Y1N,
    output Y2N,
    output Y3N,
    output Y4N,
    output Y5N,
    output Y6N,
    output Y7N
);
     integer i;
     reg [7:0] YN;
     wire[2:0] cba,G;
     
     assign cba = {C,B,A};
     assign G = {G1,G2AN,G2BN};
     
     assign Y0N = YN[0];
     assign Y1N = YN[1];
     assign Y2N = YN[2];
     assign Y3N = YN[3];
     assign Y4N = YN[4];
     assign Y5N = YN[5];
     assign Y6N = YN[6];
     assign Y7N = YN[7];
 
     always @(*) begin
        if(G == 3'b100) begin 
            for(i=0;i<=7;i=i+1)
                if(cba == i)
                    YN[i] = 0;
                else 
                    YN[i] = 1;
        end else begin
            YN = 8'b11111111;
        end
    end

endmodule

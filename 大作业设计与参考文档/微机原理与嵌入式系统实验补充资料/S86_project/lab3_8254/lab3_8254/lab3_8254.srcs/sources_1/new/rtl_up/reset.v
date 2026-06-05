`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
module reset(
    input lock,
    input button,
    output reset
    );
    
    assign reset = (!lock) | (button); 
    
endmodule

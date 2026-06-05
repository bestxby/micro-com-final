-- Copyright 1986-2015 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2015.4 (win64) Build 1412921 Wed Nov 18 09:43:45 MST 2015
-- Date        : Sat Aug 19 15:30:23 2017
-- Host        : hp-PC running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub
--               c:/Users/hp/Desktop/sysclassfiles/interface/solution/Ex_5/Ex_5_S8254/Ex_5_S8254.srcs/sources_1/ip/Decoder_0/Decoder_0_stub.vhdl
-- Design      : Decoder_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7a100tfgg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity Decoder_0 is
  Port ( 
    A : in STD_LOGIC;
    B : in STD_LOGIC;
    C : in STD_LOGIC;
    G1 : in STD_LOGIC;
    G2AN : in STD_LOGIC;
    G2BN : in STD_LOGIC;
    Y0N : out STD_LOGIC;
    Y1N : out STD_LOGIC;
    Y2N : out STD_LOGIC;
    Y3N : out STD_LOGIC;
    Y4N : out STD_LOGIC;
    Y5N : out STD_LOGIC;
    Y6N : out STD_LOGIC;
    Y7N : out STD_LOGIC
  );

end Decoder_0;

architecture stub of Decoder_0 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "A,B,C,G1,G2AN,G2BN,Y0N,Y1N,Y2N,Y3N,Y4N,Y5N,Y6N,Y7N";
attribute X_CORE_INFO : string;
attribute X_CORE_INFO of stub : architecture is "Decoder,Vivado 2015.4";
begin
end;

-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
-- Date        : Mon Aug  5 09:45:57 2024
-- Host        : LAPTOP-42E9R2TH running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub
--               e:/EES-351-V1.1/project/lab1_io/lab1_io.srcs/sources_1/ip/S86_0/S86_0_stub.vhdl
-- Design      : S86_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7z020clg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity S86_0 is
  Port ( 
    wb_clk_i : in STD_LOGIC;
    wb_rst_i : in STD_LOGIC;
    wb_dat_i : in STD_LOGIC_VECTOR ( 15 downto 0 );
    wb_dat_o : out STD_LOGIC_VECTOR ( 15 downto 0 );
    wb_adr_o : out STD_LOGIC_VECTOR ( 19 downto 1 );
    wb_we_o : out STD_LOGIC;
    wb_tga_o : out STD_LOGIC;
    wb_sel_o : out STD_LOGIC_VECTOR ( 1 downto 0 );
    wb_stb_o : out STD_LOGIC;
    wb_cyc_o : out STD_LOGIC;
    wb_ack_i : in STD_LOGIC;
    wb_tgc_i : in STD_LOGIC;
    wb_tgc_o : out STD_LOGIC;
    nmi : in STD_LOGIC;
    nmia : out STD_LOGIC;
    pc : out STD_LOGIC_VECTOR ( 19 downto 0 )
  );

end S86_0;

architecture stub of S86_0 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "wb_clk_i,wb_rst_i,wb_dat_i[15:0],wb_dat_o[15:0],wb_adr_o[19:1],wb_we_o,wb_tga_o,wb_sel_o[1:0],wb_stb_o,wb_cyc_o,wb_ack_i,wb_tgc_i,wb_tgc_o,nmi,nmia,pc[19:0]";
attribute X_CORE_INFO : string;
attribute X_CORE_INFO of stub : architecture is "zet,Vivado 2019.1";
begin
end;

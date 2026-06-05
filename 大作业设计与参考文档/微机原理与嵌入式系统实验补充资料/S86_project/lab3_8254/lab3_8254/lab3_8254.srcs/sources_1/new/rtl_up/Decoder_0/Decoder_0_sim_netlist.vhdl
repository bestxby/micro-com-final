-- Copyright 1986-2015 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2015.4 (win64) Build 1412921 Wed Nov 18 09:43:45 MST 2015
-- Date        : Sat Aug 19 15:30:23 2017
-- Host        : hp-PC running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode funcsim
--               c:/Users/hp/Desktop/sysclassfiles/interface/solution/Ex_5/Ex_5_S8254/Ex_5_S8254.srcs/sources_1/ip/Decoder_0/Decoder_0_sim_netlist.vhdl
-- Design      : Decoder_0
-- Purpose     : This VHDL netlist is a functional simulation representation of the design and should not be modified or
--               synthesized. This netlist cannot be used for SDF annotated simulation.
-- Device      : xc7a100tfgg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Decoder_0_Decoder is
  port (
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
  attribute ORIG_REF_NAME : string;
  attribute ORIG_REF_NAME of Decoder_0_Decoder : entity is "Decoder";
end Decoder_0_Decoder;

architecture STRUCTURE of Decoder_0_Decoder is
begin
Y0N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFFFFFFFB"
    )
        port map (
      I0 => G2AN,
      I1 => G1,
      I2 => G2BN,
      I3 => B,
      I4 => A,
      I5 => C,
      O => Y0N
    );
Y1N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFFFFFFBF"
    )
        port map (
      I0 => G2BN,
      I1 => G1,
      I2 => A,
      I3 => B,
      I4 => G2AN,
      I5 => C,
      O => Y1N
    );
Y2N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFFFFFFBF"
    )
        port map (
      I0 => G2BN,
      I1 => G1,
      I2 => B,
      I3 => C,
      I4 => G2AN,
      I5 => A,
      O => Y2N
    );
Y3N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFFFFFF7F"
    )
        port map (
      I0 => G1,
      I1 => A,
      I2 => B,
      I3 => C,
      I4 => G2BN,
      I5 => G2AN,
      O => Y3N
    );
Y4N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFFFFFFBF"
    )
        port map (
      I0 => G2BN,
      I1 => G1,
      I2 => C,
      I3 => B,
      I4 => G2AN,
      I5 => A,
      O => Y4N
    );
Y5N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFFFFFF7F"
    )
        port map (
      I0 => G1,
      I1 => A,
      I2 => C,
      I3 => B,
      I4 => G2BN,
      I5 => G2AN,
      O => Y5N
    );
Y6N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFFFFFF7F"
    )
        port map (
      I0 => G1,
      I1 => C,
      I2 => B,
      I3 => A,
      I4 => G2BN,
      I5 => G2AN,
      O => Y6N
    );
Y7N_INST_0: unisim.vcomponents.LUT6
    generic map(
      INIT => X"FFFFFFFFFF7FFFFF"
    )
        port map (
      I0 => G1,
      I1 => A,
      I2 => C,
      I3 => G2AN,
      I4 => B,
      I5 => G2BN,
      O => Y7N
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Decoder_0 is
  port (
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
  attribute NotValidForBitStream : boolean;
  attribute NotValidForBitStream of Decoder_0 : entity is true;
  attribute CHECK_LICENSE_TYPE : string;
  attribute CHECK_LICENSE_TYPE of Decoder_0 : entity is "Decoder_0,Decoder,{}";
  attribute CORE_GENERATION_INFO : string;
  attribute CORE_GENERATION_INFO of Decoder_0 : entity is "Decoder_0,Decoder,{x_ipProduct=Vivado 2015.4,x_ipVendor=xilinx.com,x_ipLibrary=user,x_ipName=Decoder,x_ipVersion=1.0,x_ipCoreRevision=2,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED}";
  attribute DowngradeIPIdentifiedWarnings : string;
  attribute DowngradeIPIdentifiedWarnings of Decoder_0 : entity is "yes";
  attribute X_CORE_INFO : string;
  attribute X_CORE_INFO of Decoder_0 : entity is "Decoder,Vivado 2015.4";
end Decoder_0;

architecture STRUCTURE of Decoder_0 is
  attribute black_box : string;
  attribute black_box of inst : label is "1";
begin
inst: entity work.Decoder_0_Decoder
     port map (
      A => A,
      B => B,
      C => C,
      G1 => G1,
      G2AN => G2AN,
      G2BN => G2BN,
      Y0N => Y0N,
      Y1N => Y1N,
      Y2N => Y2N,
      Y3N => Y3N,
      Y4N => Y4N,
      Y5N => Y5N,
      Y6N => Y6N,
      Y7N => Y7N
    );
end STRUCTURE;

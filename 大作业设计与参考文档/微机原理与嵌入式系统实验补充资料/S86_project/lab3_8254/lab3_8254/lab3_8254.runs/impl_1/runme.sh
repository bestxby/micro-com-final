#!/bin/sh

# 
# Vivado(TM)
# runme.sh: a Vivado-generated Runs Script for UNIX
# Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
# 

echo "This script was generated under a different operating system."
echo "Please update the PATH and LD_LIBRARY_PATH variables below, before executing this script"
exit

if [ -z "$PATH" ]; then
  PATH=D:/LenovoSoftstore/Vivado2019/SDK/2019.1/bin;D:/LenovoSoftstore/Vivado2019/Vivado/2019.1/ids_lite/ISE/bin/nt64;D:/LenovoSoftstore/Vivado2019/Vivado/2019.1/ids_lite/ISE/lib/nt64:D:/LenovoSoftstore/Vivado2019/Vivado/2019.1/bin
else
  PATH=D:/LenovoSoftstore/Vivado2019/SDK/2019.1/bin;D:/LenovoSoftstore/Vivado2019/Vivado/2019.1/ids_lite/ISE/bin/nt64;D:/LenovoSoftstore/Vivado2019/Vivado/2019.1/ids_lite/ISE/lib/nt64:D:/LenovoSoftstore/Vivado2019/Vivado/2019.1/bin:$PATH
fi
export PATH

if [ -z "$LD_LIBRARY_PATH" ]; then
  LD_LIBRARY_PATH=
else
  LD_LIBRARY_PATH=:$LD_LIBRARY_PATH
fi
export LD_LIBRARY_PATH

HD_PWD='C:/Users/lily/Desktop/123/lab3_8254/lab3_8254/lab3_8254.runs/impl_1'
cd "$HD_PWD"

HD_LOG=runme.log
/bin/touch $HD_LOG

ISEStep="./ISEWrap.sh"
EAStep()
{
     $ISEStep $HD_LOG "$@" >> $HD_LOG 2>&1
     if [ $? -ne 0 ]
     then
         exit
     fi
}

# pre-commands:
/bin/touch .init_design.begin.rst
EAStep vivado -log S86_top.vdi -applog -m64 -product Vivado -messageDb vivado.pb -mode batch -source S86_top.tcl -notrace



#!/bin/bash

echo "Script is being run by: $(whoami)"

if [ -z "$1" ]; then
    echo "no synName was provided was provided. usage $1 <path <source_file.cpp>"
    exit 1
fi

if [ -z "$2" ]; then
    echo "no verilog file was provided was provided. usage $2 <path <source_file.cpp>"
    exit 1
fi

SYNTHESIS_NAME=$1
VERILOG_PATH=$2

# change directory
cd ../synthesisRunner
# prepare tcl file
cp tclBase.tcl generatedTcl/$SYNTHESIS_NAME.tcl
sed  's/PROJECT_NAME/'$SYNTHESIS_NAME'/g' tclBase.tcl > generatedTcl/$SYNTHESIS_NAME.tcl

echo "verilog path is $VERILOG_PATH "
sed  -i 's#VERILOG_PATH#'$VERILOG_PATH'#g' generatedTcl/$SYNTHESIS_NAME.tcl

# launch vivado
cd project
source /tools/Xilinx/Vivado/2024.1/settings64.sh
pwd
vivado -mode batch -source ../generatedTcl/$SYNTHESIS_NAME.tcl
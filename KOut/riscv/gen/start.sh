#!/bin/bash

if [ -z "$1" ]; then
  echo "no optimization level provided. usagea $3 <-O3>"
  exit 1
fi

rm -r obj_dir
SRC_OPT_LEVEL=$1


verilator --threads 1 -O3 --cc --exe -j 1  sim_main.cpp top.v 
make OPT_FAST="$SRC_OPT_LEVEL" OPT_GLOBAL="$SRC_OPT_LEVEL" -j -C obj_dir -f Vtop.mk Vtop
cd obj_dir
./Vtop
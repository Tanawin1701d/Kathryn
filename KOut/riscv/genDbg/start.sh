#!/bin/bash
verilator --threads 1 -O3 --cc --trace --exe  -j 1  sim_main.cpp top.v
make -j -C obj_dir -f Vtop.mk Vtop
cd obj_dir
./Vtop
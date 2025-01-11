#!/bin/bash
verilator --threads 1 -O3 --cc --trace --exe  -j 1  sim_main.cpp top.v

# cd obj_dir
# ./Vtop
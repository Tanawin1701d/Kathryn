#!/bin/bash
verilator --threads 1 -O3 --cc --trace --exe -CFLAGS "-O3" -LDFLAGS "-O3"  -j 1  sim_main.cpp top.v 

#!/bin/bash


START_TC_NUM=5
STOP_TC_NUM=23

for x in  $(seq $START_TC_NUM $STOP_TC_NUM)
do
    iverilog -o program/test$x.vvp testbench/testBench$x.v model/test$x.v
    echo "-------------TESTCASE $x-------------"
    vvp program/test$x.vvp
done
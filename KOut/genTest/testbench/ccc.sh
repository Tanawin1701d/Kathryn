#!/bin/bash


START_TC_NUM=19
STOP_TC_NUM=23

for x in  $(seq $START_TC_NUM $STOP_TC_NUM)
do
    cp testBench18.v testBench$x.v
    sed -i 's@output/18.vcd@output/'$x'.vcd@g' testBench$x.v
done
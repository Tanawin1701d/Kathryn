#!/bin/bash


START_TC_NUM=10
STOP_TC_NUM=17

for x in  $(seq $START_TC_NUM $STOP_TC_NUM)
do
    cp testBench9.v testBench$x.v
    sed -i 's@output/9.vcd@output/'$x'.vcd@g' testBench$x.v
done
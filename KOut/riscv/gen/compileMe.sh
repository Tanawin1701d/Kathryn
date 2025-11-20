#!/bin/bash
make -j -C obj_dir -f Vtop.mk Vtop
cd obj_dir
./Vtop
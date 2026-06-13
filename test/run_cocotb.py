#!/usr/bin/env python3
# Single entry point: discover every tc*.py under test/model (each registers
# itself into the pool on import), then build + simulate them all with cocotb's
# Python runner API. No Makefile.
#
#   PYTHONPATH=py .venv/bin/python test/run_cocotb.py [simulator [tc_name ...]]
#
# `simulator` defaults to "icarus" (iverilog). VCDs land in
# test/.model_output/<case>/sim_build/.
#
# Examples:
#   python test/run_cocotb.py                         # run all
#   python test/run_cocotb.py icarus                  # run all, explicit sim
#   python test/run_cocotb.py icarus tc2_par          # only tc2_par
#   python test/run_cocotb.py icarus tc2_par tc3_sif  # tc2_par and tc3_sif

import sys
import pathlib

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))  # find cocotb_pool

import cocotb_pool

if __name__ == "__main__":
    sim    = sys.argv[1] if len(sys.argv) > 1 else "icarus"
    filter = sys.argv[2:] if len(sys.argv) > 2 else []
    cocotb_pool.discover_and_run(sim, filter or None)

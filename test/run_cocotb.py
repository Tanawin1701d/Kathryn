#!/usr/bin/env python3
# Single entry point: discover every tc*.py under test/model (each registers
# itself into the pool on import), then build + simulate them all with cocotb's
# Python runner API. No Makefile.
#
#   PYTHONPATH=py .venv/bin/python test/run_cocotb.py [simulator]
#
# `simulator` defaults to "icarus" (iverilog). VCDs land in
# test/.model_output/<case>/sim_build/.

import sys
import pathlib

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))  # find cocotb_pool

import cocotb_pool

if __name__ == "__main__":
    sim = sys.argv[1] if len(sys.argv) > 1 else "icarus"
    cocotb_pool.discover_and_run(sim)

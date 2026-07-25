#!/bin/sh
# Boot Linux on the Kathryn RV64 core (interactive console on stdin/stdout).
# Prereqs: artifacts built (build.sh all + rootfs + embed) and the linux-preset
# core+harness built (e.g. via test/riscv/run_micro.py).
set -e
cd "$(dirname "$0")/../.."

SIM=${SIM:-test/riscv/.out/rtl/sim/obj_dir/simharness}
exec "$SIM" \
    +image=tools/linux_image/artifacts/Image@80000000 \
    +image=tools/linux_image/artifacts/virt_nommu.dtb@87e00000 \
    +max-cycles=0 "$@"

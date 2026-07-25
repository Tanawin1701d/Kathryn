#!/bin/sh
# Fetch the official riscv-tests suite (pinned) into a gitignored checkout.
# We only use isa/rv64u*/ sources + isa/macros/scalar — the env/ submodule is
# replaced by our own CSR-free env in tools/riscv_tests/env/.
set -e
cd "$(dirname "$0")"

PIN=408e461da11e0b298c4b69e587729532787212f5   # riscv-software-src/riscv-tests, 2024
[ -d riscv-tests ] || git clone https://github.com/riscv-software-src/riscv-tests riscv-tests
cd riscv-tests
git fetch --quiet origin "$PIN" 2>/dev/null || true
git checkout --quiet "$PIN" 2>/dev/null || echo "fetch.sh: pin $PIN not found, using default HEAD"
git submodule update --init env      # encoding.h lives in the env submodule
echo "riscv-tests at $(git rev-parse --short HEAD)"

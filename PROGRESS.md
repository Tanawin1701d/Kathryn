# RISC-V RV64 nommu-Linux core — loop progress

Branch: `riscv-linux-core` (off `kathryn-rust-py`). Plan: `~/.claude/plans/i-want-you-to-polished-blum.md`.
Done condition: nommu Linux shell prompt on simulated UART (`ls` / `echo hi` round-trip).

## Milestones

### M0 — Infrastructure & reusable features
- [ ] M0.1 Rust signed ops (`ArithShrA`/`ArithDivS`/`ArithRemS` + DSL `.sra/.sdiv/.srem` + `lib/arith.py mulh`) — test: `PYTHONPATH=py python test/run_cocotb.py icarus tc41_signed_ops`
- [ ] M0.2 mem_blk `$readmemh` init (`init_file=`) — test: `PYTHONPATH=py python test/run_cocotb.py icarus tc42_mem_init`
- [ ] M0.3 lib select helpers (`muxN`/`decoder`/`priority_encoder`) — test: `PYTHONPATH=py python test/run_cocotb.py icarus tc43_lib_select`
- [ ] M0.4 Verilator C++ harness (`tools/simharness/`) — test: harness smoke on bus-echo .v
- [ ] M0 exit: full `test/run_cocotb.py` regression green (tc1–tc43)

### M1 — RV64I core
- [ ] riscv/ package skeleton (config/isa/bus/regfile/decode/alu/lsu/core/soc/gen)
- [ ] Unit tests tcr* green on Icarus (dsl-mem config)
- [ ] tools/riscv_tests fetch+build+run.py
- [ ] All `rv64ui-p-*` pass on Verilator harness; MIPS measured

### M2 — Zicsr / traps / CLINT
- [ ] csr.py + trap FSM states + interrupt sampling
- [ ] `rv64mi-p-*` pass (skips documented); timer-irq payload passes

### M3 — M + A extensions
- [ ] `rv64um-p-*` pass
- [ ] `rv64ua-p-*` pass

### M4 — SoC contract + bare-metal payloads
- [ ] virt-nommu memory map + DTS; boot_regs preset
- [ ] DSL periph clint/uart16550 (cocotb-tested)
- [ ] payloads: hello_uart, timer_irq, echo, memtest pass; interactive echo works

### M5 — Linux boot
- [ ] tools/linux_image/build.sh (prebuilt-first, cached)
- [ ] M5a: kernel boots to "Run /init" panic
- [ ] M5b: shell prompt; `ls`/`echo hi` round-trip  ← DONE condition

### M6 (stretch) — C extension front-end, iterative muldiv, pipeline

## Current task
M0.1 Rust signed ops

## Blockers
(none)

## Decision log
- 2026-07-25: Plan approved. Branch `riscv-linux-core` created.

## Iteration log
- 2026-07-25: loop start.

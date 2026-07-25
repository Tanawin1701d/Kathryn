# RISC-V RV64 nommu-Linux core — loop progress

Branch: `riscv-linux-core` (off `kathryn-rust-py`). Plan: `~/.claude/plans/i-want-you-to-polished-blum.md`.
Done condition: nommu Linux shell prompt on simulated UART (`ls` / `echo hi` round-trip).

## Milestones

### M0 — Infrastructure & reusable features
- [x] M0.1 Rust signed ops (`ArithShrA`/`ArithDivS`/`ArithRemS` + DSL `.sra/.sdiv/.srem` + `lib/arith.py mulh`) — test: `PYTHONPATH=py python test/run_cocotb.py icarus tc41_signed_ops`
- [x] M0.2 mem_blk `$readmemh` init (`init_file=`) — test: `PYTHONPATH=py python test/run_cocotb.py icarus tc42_mem_init`
- [x] M0.3 lib select helpers (`muxn`/`decoder`/`priority_encoder`) — test: `PYTHONPATH=py python test/run_cocotb.py icarus tc43_lib_select`
- [x] M0.4 Verilator C++ harness (`tools/simharness/`) — test: `.venv/bin/python tools/simharness/smoke/run.py`
- [x] M0 exit: full `test/run_cocotb.py` regression green (82/82 incl. tc41–43); pytest smoke 57/57

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
M1: riscv/ package skeleton (config/isa/bus/regfile/decode/alu/lsu/core/soc/gen)

## Blockers
(none)

## Decision log
- 2026-07-25: Plan approved. Branch `riscv-linux-core` created.
- 2026-07-25: Bus contract uses addr+size (no wstrb); data in low lanes both directions. 2 cycles/access.
- 2026-07-25: Harness verilates with `--prefix Vdut` so C++ never sees the mangled Kathryn top name.
- 2026-07-25: mulh done via 128-bit widening in Python lib (no new Rust mul op needed) — tc41 confirms.
- 2026-07-25: PLIC likely needed at M4/M5 (nommu_virt kernel uses SiFive PLIC for UART irq) — C++ model, decide at M4.

## Iteration log
- 2026-07-25: loop start.
- 2026-07-25: M0.1+M0.3 green (tc41/tc43) — commit cb17683.
- 2026-07-25: M0.2 green (tc42) — commit 83b6f34. Full regression 82/82.
- 2026-07-25: M0.4 harness smoke PASS (Verilator 5.48 wheel).

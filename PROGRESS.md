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

### M1 — RV64I core  ✅
- [x] riscv/ package (config/isa/core/gen; decode+ALU as comb sections inside RV64Core — no submodules needed yet)
- [x] Micro tests via harness — `.venv/bin/python test/riscv/run_micro.py` (tcr01 PASS)
- [x] tools/riscv_tests fetch+build+run.py (custom CSR-free env until M2)
- [x] All rv64ui pass — `.venv/bin/python tools/riscv_tests/run.py --suite rv64ui` → **52/52**
- [x] Speed: 17.7 Mcyc/s sustained (trace-built binary) ≈ 2.7 MIPS at CPI≈6.5 → Linux boot ≈ minutes. GO.

### M2 — Zicsr / traps / CLINT  ✅
- [x] riscv/csr.py (M-mode CSRs, read mux, strobe-gated write chain, trap bundle) + trap/mret FSM branches
- [x] rv64mi **14/16** — `tools/riscv_tests/run.py --suite rv64mi --preset rv64i_zicsr`
      (skips: `access` needs bus access faults, `breakpoint` needs debug-trigger CSRs — neither needed for nommu Linux)
- [x] rv64ui still 52/52 on zicsr preset; timer-irq payload tcr02 PASS (CLINT → mtip → trap → mret)

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
M3: M extension (mul/div via lib.arith) then A extension (LR/SC + AMOs) — exit: rv64um + rv64ua suites

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
- 2026-07-25: M1 bug 1: cif chain samples conditions at the instr-latch posedge → 1-cycle decode-settle (sywait(1)) after latch.
- 2026-07-25: M1 bug 2 (DSL, reusable fix): sub-slicing a slice view was absolute, not relative — broke lib.sext sign-bit on imm decode. Fixed in signal.py (commit 170d3da).
- 2026-07-25: M1 DONE — rv64ui 52/52 on Verilator harness; 17.7 Mcyc/s sustained (commit pending).
- 2026-07-25: Decisions: unit tests run via harness (not cocotb/DSL-RAM) — uniform + avoids sub-word DSL RAM; core is a single Module (comb sections, no submodule hierarchy) for readability; riscv-tests use custom CSR-free env until M2 lands Zicsr.

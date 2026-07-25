# simharness — raw Verilator C++ harness for Kathryn-emitted SoCs

cocotb's per-cycle callbacks are far too slow for long workloads (an OS boot is
~10^9 cycles). This harness verilates an emitted-Verilog directory into a native
executable whose C++ side models the memory system, so simulation runs at
Verilator speed with instant binary image loading.

## Build

```sh
python tools/simharness/build.py --verilog-dir <emitted-dir> [--top NAME] [--trace] [--out DIR]
# → <out>/obj_dir/simharness
```

`--top` defaults to the `module` name found in `<verilog-dir>/top.v`. The model
is always verilated with `--prefix Vdut`, so the C++ side is DUT-name-agnostic.

## DUT port contract

The top module must expose exactly these ports (Kathryn side: `mark_input` /
`mark_output` with these IO names; `clk`/`mrst` come from the Kathryn build):

```
input  clk, mrst                      // mrst active-high
output mem_req, mem_we                // request valid / write-enable
output [63:0] mem_addr, mem_wdata     // byte address, write data (low lanes)
output [1:0]  mem_size                // 0=1B 1=2B 2=4B 3=8B
input  mem_ack                        // access served
input  [63:0] mem_rdata               // read data (low lanes)
input  mtip, msip, meip               // timer / software / external interrupt
```

Protocol (2 cycles per access): hold `mem_req` with stable payload until `mem_ack`
is sampled high at a posedge, then deassert `mem_req` for at least one cycle.
The harness serves the access at the posedge where it samples `req=1, ack=0` and
presents `rdata`/`ack` for the following cycle; `ack` clears once `req` falls.

## Devices / memory map (plusargs, defaults in parens)

| Device | Plusargs |
|--------|----------|
| RAM    | `+ram-base=` (0x80000000) `+ram-size=` (0x8000000 = 128 MB) |
| CLINT  | `+clint-base=` (0x2000000): msip@+0, mtimecmp@+0x4000, mtime@+0xBFF8 |
| UART   | `+uart-base=` (0x10000000): 8250, 1-byte regs, TX→stdout, stdin→RX |
| HTIF   | `+tohost=<addr>` (off): riscv-tests exit — 1=pass, odd n=fail (n>>1) |

Numeric plusargs use strtoull base 0 — write hex with an explicit `0x` prefix.
Other flags: `+image=<file>@<hexaddr>` (repeatable, raw binary loaded into RAM;
the `@addr` part is always parsed as hex),
`+max-cycles=N` (0 = unlimited; exit code 124 on timeout), `+reset-cycles=N` (4),
`+vcd=<file>` (needs `--trace` build), `+quiet` (suppress harness banners).

Exit codes: 0 pass (tohost=1), n fail (tohost odd, n = tohost>>1), 124 watchdog.

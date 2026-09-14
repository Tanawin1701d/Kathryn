# Kathryn2

A cycle-accurate control-flow and resource-abstraction HDL framework. Describe
control flow and hardware resources at a high level in Python, keep full
cycle-accurate control, and emit clean Verilog from a Rust core.

- **You declare** the registers, wires and modules, and write the control flow
  as nested blocks.
- **Kathryn generates** the state machines, the `always` block that merges
  every write to a register, the pipeline handshakes, and the ports between
  modules.
- **Every register and clock edge** in the output is one you declared.
- **This repository** is the Python + Rust implementation. See
  [Status](#status).

```python
from kathryn import *

class Hello(Module):
    @init
    def declare(self):
        self.x = reg(8, "x")
        self.y = reg(8, "y")

    @flow
    def run(self):
        with seq():                 # two clocked steps, one after the other
            self.x |= 48
            self.y |= self.x

reset()
build_model(Hello())
emit_verilog("out")                 # out/top.v and out/sim_manifest.json
```

## The three abstractions

### 1. Hybrid Design Flow (HDF)

An abstract model for hardware control flow.

- Sequential, parallel and conditional blocks, plus the pipeline halves `pip` /
  `zync`, are plain Python context managers that compile to explicit state
  machines.
- One flow can be an FSM in one place and flat RTL in the next.
- The block prefix says what it costs: `s` takes a clocked step, `c` reads its
  condition in the same cycle, `z` adds no state, and a statement outside any
  block is a plain `always`.

| Blocks                                          | Meaning                                    |
| ----------------------------------------------- | ------------------------------------------ |
| `seq` `par`                                     | clocked steps / fork and join              |
| `sif` `cif` `zif` (+ `cselif` `cselse` `zelif` `zelse`) | if chain: clocked / same cycle / zero cycle |
| `zstate` `zcase`                                | zero-cycle switch on a state signal        |
| `swhile` `cwhile` `cdowhile` `cloop`            | loops                                      |
| `scwait` `sywait`                               | wait for a condition / for n cycles        |
| `pick` `pif` `pidef`                            | multi-way select                           |
| `pip` `zync`                                    | pipeline stage with its handshake          |

```python
@flow
def main(self):                     # inputs start, x; output busy; regs acc, n, run
    self.busy *= self.run                       # flat RTL: always @(*)
    with zif(self.start):                       # zero-cycle gate, no state
        self.run |= 1
    with seq():                                 # FSM: one state bit per step
        scwait(self.start)
        self.acc |= 0
        with swhile(self.n < 4):                # condition costs a step
            self.acc |= self.acc + self.x
            self.n   |= self.n + 1
        with cif(self.acc > 100):               # condition read the same cycle
            self.acc |= 100
        self.run |= 0
```

- Generated: the state bits, the wait node, the loop's terminal test.
- Declared: the registers and wires, exactly the ones you wrote.

The pipeline halves, `pip` (granter) and `zync` (requester), build a stage and
its handshake:

```python
@flow
def stages(self):                   # regs a, b, c; hold; self.arb = [PipCon() for _ in range(4)]
    with pip(self.arb[0], auto_req=True):       # source end: always requesting
        with zync(self.arb[1]):                 # hands off to stage 2
            self.a |= self.a + 1

    with pip(self.arb[1]):                      # granted by stage 1's zync
        with seq():
            with cif(self.hold):                # a stall, five cycles
                sywait(5)
            with zync(self.arb[2]):
                self.b |= self.a

    with pip(self.arb[2]):
        with zync(self.arb[3], auto_ack=True):  # sink end: always granted
            self.c |= self.b
```

- Two adjacent stages share one `PipCon`, so stage N's `zync` is stage N+1's
  request.
- `auto_req` is the always-requesting source end, `auto_ack` the always-granted
  sink end.
- Generated: the REQ/ACK wiring, the arbiter, the per-stage state. The stall in
  stage 2 back-pressures stage 1, which freezes with it.

### 2. Decentralized Update

Centralized control logic relaxed.

- In Verilog a register has one `always` block, so every writer must be
  gathered into it.
- In Kathryn a register has no block of its own. Any block may update it.
- Each assignment, from any flow and any depth, adds an update event with its
  own condition and priority.
- Several writers to one register are legal and deterministic: conflicts
  resolve exactly as declared, with no hand-routed valid/select tree.

```python
@flow
def count(self):                    # inputs go, clr; reg cnt
    self.cnt.reset(0)                           # an event at reset priority
    with zif(self.go):
        self.cnt |= self.cnt + 1                # a user-priority event, gated on go

@flow
def clear(self):                    # written somewhere else entirely
    with zif(self.clr):
        with priority(DEFAULT_UE_PRI_USER + 1):
            self.cnt |= 0                       # outranks the increment
```

- At build the events are sorted into one `always` block where the last write
  wins: increment, then clear, then master reset.
- `reg.reset(v)` and `wire.default(v)` are events on the same ladder.
- A new writer is a new method or a new file. Nothing existing changes.

### 3. Hardware Aggregator: the Karray

One entity that bundles many hardware components.

- A `Karray` is a multi-dimensional array of records.
- Each field of each element is its own register or wire.
- A dimension is indexed by an int, a signal, or a function, and any mix works
  in one statement.

```python
class Entry(Karray):
    valid = kaf(1)
    data  = kaf(8)

class RegFile(Module):
    @init
    def declare(self):
        self.rf   = Entry(HwComponentType.REG, (4,), "rf")
        self.addr = wire(2, "addr")
        self.sel  = wire(4, "sel")
        self.rd   = reg (8, "rd")
        self.mx   = reg (8, "mx")

    @flow
    def run(self):
        with seq():
            self.rf[0].data |= 42                          # static index
            self.rf[1]      |= {"valid": 1, "data": 7}     # whole record
            self.rf[self.addr].data |= 9                   # runtime index
            self.rf[lambda i: self.sel[i]].valid |= 1      # one enable per element
            self.rd |= self.rf[self.addr].data             # runtime read: a mux tree
            self.mx |= self.rf[lambda a, b, lvl: a.fields["data"] >= b.fields["data"]].data
```

- The last line is a reduce read: the function picks the winner at each node of
  a balanced tree, so `mx` is the maximum `data` in log2(N) levels.
- Records nest like Chisel bundles, copy between arrays by field name, and
  reset per field.
- A rename table or a reorder buffer is one class and one line.

## Also in the box

- **Pipelines.** `pip` and `zync` share an arbiter; stalls, bubbles and flushes come from it.
- **Hierarchy.** A child reads a parent's signal by attribute; ports, wires, `clk` and `mrst` are generated.
- **Simulation.** cocotb tests on Icarus or Verilator; `KSim(dut)` reaches any internal signal by its model name, no output port needed. Builds are cached by source digest.
- **Builders.** `counter`, `Arb`, `mux`, `any_of`, `sum_cnt`, `rotate_left`.
- **Typed operators.** `|=` for registers, `*=` for wires; the wrong one is a `TypeError`. Int literals size themselves.
- **Rust core.** One arena owns the model; PyO3 is an optional feature.

## Status

Kathryn exists in two implementations.

- **Legacy C++.** The version in the Kathryn paper, under review at IEEE TCAD
  after major revision. Verified cycle-accurate against RIDECORE and
  synthesized on a Kria KV260. Documented in the
  [Kathryn C++ book](https://www.kathryn-tools.org/cppbook/getting-started/introduction/).
- **Python + Rust.** This repository, version 0.1.0. Under active development
  and not yet feature-complete. Verilog is the only backend.

## Built with Kathryn

- **[Carolyne](https://www.kathryn-tools.org/carolyne/)**, an ISA-agnostic
  out-of-order CPU generator, first target RV32I.

## Getting started

Needs:

- Rust, edition 2024.
- Python 3.9+.
- Icarus Verilog, or Verilator 5.036+, for simulation.

```sh
python -m venv .venv && source .venv/bin/activate
pip install maturin cocotb pytest
maturin develop                                      # builds the core into py/kathryn/
```

```sh
cargo build                                          # 0 errors expected
PYTHONPATH=py pytest py/tests                        # unit tests
PYTHONPATH=py python test/run_cocotb.py              # every design under Icarus
PYTHONPATH=py python test/run_cocotb.py verilator tc20_pip_zync_baseline
```

- Each `test/model/tc*.py` is one design plus its cocotb test.
- No Makefile.

## Layout

| Path                   | Contents                                              |
| ---------------------- | ----------------------------------------------------- |
| `src/model/`           | the model: arena, components, flow blocks, karray     |
| `src/backends/`        | the Verilog emitter and cross-module routing          |
| `src/applications/py/` | PyO3 bindings                                         |
| `py/kathryn/`          | the Python DSL; `sim/` holds `KSim` and the runner    |
| `test/model/`          | 42 cocotb test designs                                |

## Links

- **Website**: <https://www.kathryn-tools.org/>
- **Userbook** (install, signals, flow control): <https://www.kathryn-tools.org/userbook/getting-started/introduction/>
- **Devbook** (compiler internals): <https://www.kathryn-tools.org/devbook/architecture/overview/>
- **Kathryn C++ book** (legacy): <https://www.kathryn-tools.org/cppbook/getting-started/introduction/>
- **Source**: <https://github.com/Tanawin1701d/Kathryn>

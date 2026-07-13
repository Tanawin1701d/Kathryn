# CLAUDE.md

Guidance for working in the **Kathryn** repository.

## What Kathryn is (positioning)

Kathryn is a **framework-assisted Hardware Description Language embedded in C++** — conceptually in the same family as Chisel / PyMTL / PyRTL / Spade, *not* an HLS tool and *not* SystemC. A designer writes ordinary C++ classes; the framework records the described hardware into an in-memory **model**, which can then be driven down **two independent backends**:

1. **The modeling framework / language** (`src/model/`) — the C++-embedded DSL the designer writes (macros like `mReg`, `mWire`, blocks like `seq`, `par`, `zif`, `pip`). Building a model is pure elaboration: it constructs an in-memory graph of hardware components and control-flow blocks.
2. **The Hybrid Simulator (HS)** (`src/sim/`) — a cycle-accurate, event-driven simulator. It **generates optimized C++ from the model, compiles it to a `.so`, and `dlopen`s it** for fast execution. Also produces VCD waveforms and the ZEP profiler report.
3. **The HDL generator** (`src/gen/`) — lowers the same model to synthesizable **Verilog**.

> These three concerns are deliberately separate layers. When editing, keep them separate: a change to a hardware primitive (`src/model/hwComponent/...`) usually needs a matching change in its **sim proxy** (`src/sim/modelSimEngine/hwComponent/.../*Sim.cpp`) *and* its **gen proxy** (`src/gen/proxyHwComp/.../*Gen.cpp`). These three mirror each other component-for-component.

This layering distinction is exactly what TCAD Reviewer #2 asked to be made explicit — keep it crisp in code and prose.

## Build & run

Standard build (RIDECORE co-simulation OFF — the common case):
```bash
mkdir -p build && cd build
cmake -DBUILD_RIDECORE=OFF ..
make -j
./Kathryn ../params/<someParams>
```

- The single executable `Kathryn` takes **one argument: a params file** (see `params/`). `main.cpp` → `readParamKathryn()` → `start()` in `src/frontEnd/cmd/cfe.cpp`.
- `cfe.cpp::start()` dispatches on the `testType` key in the params file. Map of `testType` → entry point:
  | `testType` | runs |
  |---|---|
  | `testSimple` | auto sim regression (`src/test/autoSim`) |
  | `testRiscv` / `testRiscvSort` | in-order RISC-V sim |
  | `testGenRiscv` | in-order RISC-V → Verilog |
  | `testO3Sim` / `testGenO3` | Kride OoO CPU sim / → Verilog |
  | `testKrideSim` | Kride standalone sim |
  | `testRideSim` | RIDECORE via Verilator (needs `BUILD_RIDECORE=ON`) |
  | `testKrideRideCombSim` | **co-simulation** Kride vs RIDECORE, cycle-by-cycle compare |
  | `testGen` | small generator test cases |

- **RIDECORE co-sim** (`BUILD_RIDECORE=ON`) requires the `extSim/ridecore` git submodule populated and Verilator installed; it compiles `extSim/ridecore/src/fpga` and links `verilator_ridecore`. RIDECORE is a submodule (`git submodule update --init`).
- `blinkSample.cpp` is a standalone minimal example (its own `main`); to build it, uncomment it in `add_executable` in `CMakeLists.txt` and comment out `main.cpp`. Read it first to learn the API — it is the smallest complete design+sim.
- Params files are simple `key = value` (`;` comments) parsed by `src/frontEnd/cmd/paramReader.cpp`. Common keys: `prefix` (output dir), `vcdFile`, `profFile`, `genFolder`, `topFileName`, `topModName`, `buildSimMode` (e.g. `gcr` = **g**enerate/**c**ompile/**r**un the sim `.so`), `limitCycle`, `workload`.
- Generated artifacts land under `KOut/<subdir>/`. The JIT sim `.so` build happens in `modelCompile/` via `modelCompile/startGen.sh` (`g++ -fPIC -shared -O3 ...`), output to `modelCompile/build/*.so`, generated source in `modelCompile/generated/`.

There is no test runner beyond `./Kathryn ../params/smParams` (which runs the `src/test/autoSim` suite). Prefer that as a smoke test after changes.

## Core concepts / terminology

Terminology in the paper is inconsistent (Reviewer #8) — this is the canonical mapping:

- **HDF — Hybrid Design Flow**: the overall control-flow abstraction methodology.
- **HDB — Hybrid Design Block**: an individual control-flow construct (`seq`, `par`, `zif`, `pip`, …). "Hybrid Design Block" and "HDB" are the same thing; prefer "HDB".
- **CCE — Cycle-Considered Element**: the blocking update operator **`<<=`**. Each `<<=` is one cycle of state advance; combined with HDBs this is how cycle-accuracy is expressed in userland. `=` is the non-blocking / combinational assignment.
- **Decentralized update / decentralized control**: assignments do *not* mutate state directly. Each `<<=`/`=` pushes an **UpdateEvent** into the target component's `UpdatePool` (see `hwComponent/abstract/updateEvent.h`, `assMetaMng.h`). Priorities/conditions on events are later resolved — by the sim engine (execution order) and by the gen engine (into `if`/`case`/mux Verilog). This is what lets multiple flow blocks write the same resource without a central FSM.
- **Hardware Aggregator**: the collection abstractions in `src/model/hwCollection/` — `Slot`/`SlotMeta`, `RegSlot`/`WireSlot`, `Table`, `MemTable`, `mux`. Used heavily in Kride (reservation stations, ROB, register files).
- **ZEP — Zero Effort cycle-spent Profiler**: automatic per-flow-block cycle accounting, emitted by the sim (`src/sim/simResWriter/`, `FlowWriter`).
- **Kride**: the RISC-V Out-of-Order superscalar CPU case study built in Kathryn (`src/example/o3/`), modeled after **RIDECORE** (the reference Verilog OoO CPU in `extSim/ridecore/`).

## Repository map

```
main.cpp / src/kathryn.{h,cpp}      entry + lifecycle (startModel / startGen / reset)
src/frontEnd/cmd/                    param file reader + start() dispatch (cfe.cpp)

src/model/                          == THE LANGUAGE / MODEL LAYER (designer-facing) ==
  hwComponent/                        primitives: module, register, wire, value(Val/PmVal),
                                      expression, nest, memBlock, box, globalComponent
    abstract/                         Assignable, operable (operators), updateEvent, assMetaMng,
                                      makeComponent.h (the mReg/mWire/mMod/... macros),
                                      WireMarker (I/O tagging), globPool, clock/asm mode
  flowBlock/                          the HDBs -> compiled to FSM node graphs
    abstract/  nodes/  spReg/         base classes; node types; state/cnt/wait/sync special regs
    cond/(if,zif,elif) loop/(while,loop,cbreak) par/ pick/ pipeline/(pipe,zync,syncMeta)
    pipeStream/ seq/ state/(ztate,zcase) time/(wait)
  hwCollection/dataStructure/         aggregators: table/ memTable/ slot/ mux/ indexing/
  controller/                         ModelController: elaboration lifecycle, component registry,
                                      flow-block stack; clockMode / asmMode contexts

src/sim/                            == THE SIMULATOR ==
  controller/simController.*          cycle loop, neg/pos edge phases
  event/                              eventBase, eventQ (priority queue), userEvent, ctTrigEvent
  interface/simInterface.*            user subclasses this to write a testbench
  modelSimEngine/                     per-component *Sim proxies (mirror src/model), proxy build mng
    base/                             proxyBuildMng (gen C++ -> compile .so -> dlopen), simValType
    hwComponent/... flowBlock/...     wireSim/registerSim/expressionSim/moduleSim/... + probers
  logicRep/                           runtime value representation (ValRep, UintX<N>)
  simResWriter/                       VCD writer + ZEP FlowWriter (profiler)

src/gen/                            == THE VERILOG GENERATOR ==
  controller/genController.*          initEle -> routeIo -> generateEveryModule
  proxyHwComp/                        per-component *Gen proxies (mirror src/model)
    abstract/                         logicGenBase, AssignGen, updateEvent (event->Verilog engines)
    module/moduleRouting.cpp          hierarchical I/O auto-wiring (AIP_/AOP_/ABD_ wires)
    module/moduleWrite.cpp            emits the .v file (pool-ordered: reg/wire/expr/val/ops)
    register/ wire/ expression/ value/ memBlock/
src/util/fileWriter/codeWriter/     verilogWriter (CbAlways/CbIf/CbSwitch), cppWriter, fileWriter

src/example/                        == DESIGNS ==
  o3/            Kride: OoO superscalar RISC-V (core/, generation/, simulation/, simCompare/)
  riscv/         simple in-order 5-stage RISC-V (fetch/decode/execute/writeBack)
  cacheAc/  uart/  dataStruct/       smaller examples (accelerator, serial IO, queue/field)

extSim/ridecore/                    reference Verilog OoO CPU (git submodule) + counting scripts
modelCompile/                       JIT scratch area for the sim .so (generated/, build/, startGen.sh)
KOut/                               all generated output (per-example subdirs)
params/                             run configs (one per scenario)
synthesisRunner/                    Vivado TCL + launch script for synthesizing generated Verilog
```

## The Kride case study (the paper's core)

- `src/example/o3/core/` — the full OoO microarchitecture: `fetch`, `decoder`, `dispatch`, dual `execAlu`, `execBranch`, `execLdSt`, `execMul`, `rob` (64-entry), `arf`/`rrf` (arch/physical register files), reservation stations (`rsv` base, `irsv` in-order, `orsv` out-of-order, `rsvs`), `storeBuf`, `broadCast`, branch prediction (`btb`, `gshare`, `mpft` — gshare/btb currently disabled). Slot layouts in `slotParam.h`.
- `src/example/o3/generation/O3_gen.*` (`O3_GEN_MNG`) → Verilog. `src/example/o3/simulation/` (`O3_MNG`, `top.h`, probers) → standalone sim.
- `src/example/o3/simCompare/` — the equivalence harness vs RIDECORE. `simCtrlKride` (Kathryn side), `simCtrlRide`/`simCtrlComb` (RIDECORE side, needs Verilator), `simState*` capture per-cycle pipeline state and `compare()` them. Managers: `O3_MNG`, `KRIDE_MNG`, `RIDE_MNG`, `COMB_MNG`. Workloads: `standard` (10 directed tests) or `cpp` (Fibo/Tarai/Ackermann/Hanoi/Matmul/Sort/etc.).

### Metrics scripts (be precise — Reviewer #3 challenged rigor)

The productivity metric was re-derived from scratch (2026-07-09) into an **11-category,
multi-select, DOMAIN-SPLIT classification** applied to both the Kathryn C++ DSL and the
RIDECORE Verilog. It supersedes the earlier 8-category scheme. Two orthogonal axes:
**DOMAIN** (CTRL vs DATA — decided by a fixed keyword list, not judgment) × **KIND**
(HWD/CL/DT/HC), plus standalone **MD** (module/struct decl), **PARAM**, **HLH** = 11
categories: `CTRL_HWD CTRL_CL CTRL_DT CTRL_HC DATA_HWD DATA_CL DATA_DT DATA_HC MD PARAM HLH`.
Every real-code physical line gets a trailing `///<CAT> <GROUP>` tag; there is **no
priority/first-match-win** — a line is tagged for each category it qualifies for, and
multi-aspect statements are LINE-BROKEN so each line is one clean category (1/N split only
for an inseparable ctrl+data bundle, e.g. `///CTRL_HC+DATA_HC`). Firm keyword decisions:
register indices=DATA, rename-tags/rrfIdx/pointers=CONTROL, op-select fields (alu_op/funct/
imm_type/rsSel/sel_lohi/signedness/opcode)=DATA, module/struct=MD. `///DC` = excluded
(Kathryn sim instrumentation; **RIDECORE dual-issue path DUPLICATION + disabled-feature logic** — 1446 lines,
excluded to compare UNIQUE live logic once, a deliberate honest handicap to Kathryn). The full
reproducible rule set + keyword list + worked conventions + results is
`src/example/o3/count_measure` (the Reviewer-#3 artifact); gold exemplars `core/arf.h`,
`core/core.h`, `fpga/exunit_alu.v`.

Counting:
- `src/example/o3/countMeasure.py [DIR]` — per-category + per-(category×group) counts; skips `///DC`; WARNS on stray non-11-cat tags.
- `src/example/o3/countCompare.py` — side-by-side Kathryn vs RIDECORE, per-category reduction %, plus **CTRL total / DATA total rollups** (the headline).
- `src/example/o3/apply_tags.py FILE < spec` — programmatic tagger (strips old tags, PRESERVES `///DC`, appends by line#); `check_tags.py` — per-file verifier; `tagContinuation.py` — propagates a statement's tag to continuation lines (idempotent).

**Verified counts** (recompute: `python3 src/example/o3/countCompare.py`; 0 stray both sides; totals may be fractional from `+`-bundles):
- Kathryn core: CTRL_HWD 80.5, CTRL_CL 215.5, CTRL_DT 150, CTRL_HC 290, DATA_HWD 39, DATA_CL 122, DATA_DT 113, DATA_HC 178, MD 66, PARAM 298, HLH 115 → **CTRL 736, DATA 452, total 1667** (DC-excl 92). (Cycle-advancing HDB headers `pip/zync/zyncc/cwhile` = `CTRL_HWD+CTRL_CL`; mixed sub-modules `regArch/storeBuf/lss/bp` and every whole-slot ref from a mixed `SlotMeta` = `CTRL_HC+DATA_HC` bundles — each carries a control tag/rename/valid part + a data part.)
- RIDECORE fpga: CTRL_HWD 292, CTRL_CL 575.5, CTRL_DT 289, CTRL_HC 1162.5, DATA_HWD 176, DATA_CL 178.5, DATA_DT 273, DATA_HC 621.5, MD 125, PARAM 174, HLH 27 → **CTRL 2319, DATA 1249, total 3894** (DC-excl 1446). (Computed port-map args carry `+CL` in the port domain — see below — shifting ~69 half-lines from `_HC` into `_CL`; CTRL/DATA totals unaffected. PARAM includes the 151 `` `define `` constants in the `.vh` headers `alu_ops.vh`/`constants.vh`/`rv32_opcodes.vh`, symmetric with Kathryn's counted `parameter.h`/`isaParam.h`/`slotParam.h`. The CAM-wakeup module `src_manager.v` (per-RS operand forward/bypass) is now marked — initially missed. The never-instantiated latched BRAM `ram_sync.v` was moved to `fpga/deleted/` (skipped via `SKIP_DIRS`). Disabled gshare `bhr`/`prcond` plumbing in `rs_branch.v` is now `///DC`.)
- **HEADLINE control-flow reduction = CTRL total (736 vs 2319) = 68.26%.** Broken out: control connectivity CTRL_HC **75.05%** (largest — HDBs remove hand-wired control ports/handshakes), control-state decl CTRL_HWD **72.43%**, control logic CTRL_CL **62.55%**, control moves CTRL_DT **48.10%**. DATA total 63.81%; **TOTAL 57.19%**. PARAM/HLH are higher for Kathryn (embedded-C++ named params + host scaffolding) — not a regression. (Exact decimals drift slightly with edits; recompute via `countCompare.py` is the source of truth.)
- A Verilog instance port map whose arg is a computed **expression** (logic/arith op, ternary mux, concat, or bit-slice) is tagged `X_HC+X_CL` (0.5 each, same domain as the port): it both connects the port (HC) and computes the value (CL). A plain `.port(sig)` stays pure HC. Symmetric with Kathryn, where such a compute is its own CL line (a `zif` guard / `x = a & b`) rather than hidden in a port map. Applied to 138 RIDECORE lines (incl. line-broken concat continuation lines).

**This 68.26% REPLACES the old 8-category headline (CTRL 78.76% / abstraction ~78.81%).** It
is lower because the new scheme counts EVERY control-connectivity port line (CTRL_HC),
which the old narrow CTRL bucket did not — inflating RIDECORE's control denominator and
giving a more honest, reproducible figure. Report **68.26% (CTRL total)** as primary and
**CTRL_HC 75.05%** as the control-connectivity story for the dual "Control Flow + Resource
Abstraction" thesis. (The old abstraction-density ~78.81% survives only as historical context.)

Caveats: the CATEGORY totals are the reliable comparison; the GROUP axis is only loosely
aligned across the two sides (e.g. RIDECORE store buffer tagged EXEC_LDST vs Kathryn GROB).
Reviewer #1 wants **total simulation time**; Reviewer #6 wants LUT/FF/BRAM/DSP, Fmax,
critical path from synthesizing the generated Verilog (`synthesisRunner/`).

## Conventions & gotchas

- **Mirror the three layers.** Adding/altering a hardware primitive or an HDB means touching `model/`, its `sim/modelSimEngine/` proxy, and its `gen/proxyHwComp/` proxy. Forgetting one silently breaks sim or generation.
- HDBs are implemented as `for`-loop macros (`loopStMacro.h` / `LoopStMacro` CRTP). The macro body is your hardware; the loop wrapper handles enter/build/exit. Don't `return`/`break` out of an HDB body with native C++ control flow — use the provided constructs (`sbreak`, etc.).
- Component declaration macros (`mReg`, `mWire`, `mMod`, `mMem`, `mExpr`, `mVal`, `mPmVal`, `g`/`gr` for nests, `mBox`) are defined in `src/model/hwComponent/abstract/makeComponent.h` and must be used inside a `Module` context.
- Sizes: build defines `-DNOTEXCEED64` — values are ≤64-bit fast path; wider values go through `UintX<N>` in `sim/logicRep/`. Don't assume 64-bit silently everywhere.
- Generated Verilog uses systematic prefixes: `REG#`, `WIRE#`, `EXPR#`, `VAL#`, `SR_ST#` (flow-block state regs), `MEM#`, `MODULE#`, and routing wires `..._AIP_`/`_AOP_`/`_ABD_` (input/output/bridge). Output is pool-ordered, not source-ordered.
- The repo has **many `phase-*` branches**; current work is on `phase-8-tcad-major-revised`. `phase-1` is the effective main/base branch. Commit only when asked; branch off before committing on a base branch.
- Build dirs `cmake-build-debug*/` and `KOut/`, `modelCompile/build|generated/` are generated — don't hand-edit.

## Project context: TCAD major revision (as of 2026-07)

The manuscript ("KATHRYN: A Cycle-Accurate Control Flow and Resource Abstraction HDL framework …", TCAD-2026-0342) got a **major revision** (revised version due **2026-08-05**; only one major revision allowed). Code changes on this branch are likely in service of the reviews. Key asks that may drive code work:
- R1: report **total simulation time** (Kathryn vs RIDECORE).
- R2/R5: sharpen comparison/novelty vs **Chisel / SystemC / Spade / Silice / PyMTL** (mostly prose, but a real Chisel comparison may need artifacts).
- R3: rigorous, reproducible definition of the **78.81% control-flow reduction** (the `countCtrl.py` scripts + `///CTRL` convention).
- R6: **synthesis numbers** — LUT/FF/BRAM/DSP, Fmax, critical path, esp. for reservation stations / store buffer (decentralized updates & Table/Slot search). Use `synthesisRunner/` (Vivado).
- R7: discuss **generality** beyond one CPU, and the intentional non-support of multi-clock/latches/tri-state.
- R8: fix inconsistent terminology (see the glossary above) and typos.
- Editor (required): add a **generative-AI usage statement** in the acknowledgements.

# Kathryn2 — Contributor Guide for AI Agents

This file is auto-loaded by Claude Code (and most agentic dev tools) at session
start. Read it before making changes. The Rust port of Kathryn lives on the
`kathryn-rust` branch off `main`. The C++ reference implementation lives at
`/media/tanawin/tanawin1701e/project2/Kathryn` — use it as a *knowledge source*,
not a pattern source. Rust patterns deliberately diverge from C++ where pointer
ownership or inheritance does not map cleanly.

**Crate layout.** All code lives in the library crate (`src/lib.rs`); the native
binary (`src/main.rs`, bin name `kathryn_cli`) is a thin shell over it, and the
PyO3 extension (lib name `kathryn`, `crate-type = ["cdylib", "rlib"]`) is built
from the same library via maturin. PyO3 is an **optional** dependency behind the
`python` Cargo feature, so the default build compiles **zero** PyO3 macros. The
entire Python layer lives under `src/applications/` and is `#[cfg(feature =
"python")]`-gated in `lib.rs`. See §7.

---

## Table of contents

- [1. Architectural patterns](#1-architectural-patterns)
  - [1.1 ModelArena — central object store](#11-modelarena--central-object-store)
  - [1.2 Ident pattern — lightweight Copy handles](#12-ident-pattern--lightweight-copy-handles)
  - [1.3 Identifiable trait](#13-identifiable-trait)
  - [1.4 Arena factory pattern](#14-arena-factory-pattern)
  - [1.5 arena_impl files — per-category CRUD](#15-arena_impl-files--per-category-crud)
  - [1.6 Trait-object dispatch (the get-style exception)](#16-trait-object-dispatch-the-get-style-exception)
    - [1.6.1 UE take/replace_back](#161-ue-takereplace_back)
  - [1.7 Grouped index arrays](#17-grouped-index-arrays)
  - [1.8 Clock-signal policy](#18-clock-signal-policy)
- [2. Memory management mechanism](#2-memory-management-mechanism)
  - [2.1 Ownership rules](#21-ownership-rules)
  - [2.2 Lifetime / re-borrow pattern](#22-lifetime--re-borrow-pattern)
  - [2.3 Generation safety](#23-generation-safety)
  - [2.4 Reset](#24-reset)
  - [2.5 Global ID](#25-global-id)
- [3. Naming conventions](#3-naming-conventions)
  - [3.1 Files and modules](#31-files-and-modules)
  - [3.2 Types](#32-types)
    - [3.2.1 Ident variable naming — `_i` suffix](#321-ident-variable-naming--_i-suffix)
  - [3.3 Functions](#33-functions)
  - [3.4 Constants](#34-constants)
  - [3.5 Identifier prefixes (in `build_unique_name`)](#35-identifier-prefixes-in-build_unique_name)
- [4. Other things that are relevant](#4-other-things-that-are-relevant)
  - [4.1 Pre-existing build errors](#41-pre-existing-build-errors)
  - [4.2 What is *not yet* ported from C++](#42-what-is-not-yet-ported-from-c)
  - [4.3 Workflow expectations](#43-workflow-expectations)
  - [4.4 C++ reference](#44-c-reference)
  - [4.5 Memory notes](#45-memory-notes)
- [5. Backends (`src/backends/`)](#5-backends-srcbackends)
  - [5.1 Module tree](#51-module-tree)
  - [5.2 `graph.rs` — module iterator and ancestor utilities](#52-graphrs--module-iterator-and-ancestor-utilities)
  - [5.3 `io_op.rs` — IoWire helpers](#53-io_oprs--iowire-helpers)
  - [5.4 Verilog backend — `HcpBaseVb` trait](#54-verilog-backend--hcpbasevb-trait)
  - [5.5 Verilog backend — scalable dispatch in `arena_ext_vb.rs`](#55-verilog-backend--scalable-dispatch-in-arena_ext_vbrs)
  - [5.6 `routing.rs` — cross-module IO routing](#56-routingrs--cross-module-io-routing)
- [6. Code style](#6-code-style)
- [7. Python bindings (`src/applications/py/`)](#7-python-bindings-srcapplicationspy)
  - [7.1 Golden rule — the core stays PyO3-free](#71-golden-rule--the-core-stays-pyo3-free)
  - [7.2 Module tree mirrors `src/model/`](#72-module-tree-mirrors-srcmodel)
  - [7.3 Conventions](#73-conventions)
  - [7.4 Enums — single source of truth](#74-enums--single-source-of-truth)
  - [7.5 Pure-Python DSL (`py/kathryn/`)](#75-pure-python-dsl-pykathryn)

---

## 1. Architectural patterns

### 1.1 ModelArena — central object store

All long-lived model objects live in `ModelArena` (`src/model/model_arena.rs`).
Each object type is owned by a typed `ArenaGroup<T>` field on `ModelArena`:

```rust
pub(super) regs    : ArenaGroup<Reg>,
pub(super) wires   : ArenaGroup<Wire>,
pub(super) modules : ArenaGroup<Module>,
// ...
```

`ArenaGroup<T>` (`src/common/arena_base.rs`) is a generational arena. `insert`
returns an `ArenaHandle { index, generation }` and stamps that handle into the
inserted object via `Identifiable::set_arena_handle`. `get` / `get_mut` validate
the generation, so freed slots cannot be aliased.

**Never store `Box<dyn Trait>` or owning `Rc/Arc` of model objects.** Always
insert into the arena and pass `*Ident` handles around.

### 1.2 Ident pattern — lightweight Copy handles

For each arena-stored type, there is a small `Copy` ident type that callers
hold. The ident embeds an `IdentBase` which carries `global_id`, name, and the
`arena_handle`. Examples:

| Ident type           | Wraps                                                   | Located in                                                  |
| -------------------- | ------------------------------------------------------- | ----------------------------------------------------------- |
| `HcpIdent`           | `IdentBase` + `HwComponentType` + `HcpSensitiveType`    | `model/hw_component/common/hcp_ident.rs`                    |
| `NcpIdent`           | `IdentBase` + node type                                 | `model/nodes/ncp_ident.rs`                                  |
| `UpdateEventIdent`   | `IdentBase` + `UeType`                                  | `model/hw_component/common/update_event_ident.rs`           |
| `ModuleIdent`        | `IdentBase` + `master_module_handle` + `depth_level`    | `model/module/module_ident.rs`                              |

`ModuleIdent` carries two extra fields beyond `IdentBase`:
- `master_module_handle: ArenaHandle` — the parent module's arena slot; `ArenaHandle::default()` means top (no parent).
- `depth_level: u32` — nesting depth; 0 = top module, +1 per sub-module level.

`depth_level` is assigned in `stamp_module_to_parent_module` using the current trace-stack length (which equals the number of ancestor modules already on the stack, i.e. the child's depth). The stamped ident is written back into the `Module` in the arena so `module.get_ident()` stays consistent.

Idents are `Clone + Copy + Default + PartialEq + Eq` and implement
`Identifiable`. They are passed by value, never by reference where avoidable.

**Don't centralize per-type properties into a match — let each type declare its
own.** `HcpIdent` carries an `HcpSensitiveType` (`Clocked` / `Combinational` /
`ReadOnly`) telling how the component is driven and therefore how it may be
assigned. This is **not** derived by a central `match hw_type { … }`; each HW
component passes its own value at its `HcpIdent::new(hw_type, sensitive_type,
…)` call site (`Reg`/sp-regs/mem → `Clocked`, `Wire`/`IoWire` → `Combinational`,
`Val`/`Expression` → `ReadOnly`). A central switch is a maintenance trap — a new
component type silently lands in the wrong arm. Prefer the constructor-declares-
its-own-trait pattern over a re-derivation match wherever a property is intrinsic
to the type. (Python reads it via `PyHcpIdent.clocked: bool` /
`PyHcpIdent.sensitive_type: str`; `HcpSensitiveType::is_clocked()` is the single
source for the boolean.)

### 1.3 Identifiable trait

Every object stored in an arena (and every ident) implements `Identifiable`
(`src/model/common/identifier.rs`):

```rust
fn get_ident_base    (&self)     -> &IdentBase;
fn get_ident_base_mut(&mut self) -> &mut IdentBase;
fn build_unique_name (&mut self) -> &str;
```

`ArenaGroup::insert` calls `set_arena_handle` on the value via this trait, so
the object knows its own handle after insertion.

### 1.4 Arena factory pattern

Object construction goes through `make_*` / `mk_*` methods on `ModelArena`,
split across `arena_factory*.rs` files by category:

| File                          | Covers                          |
| ----------------------------- | ------------------------------- |
| `arena_factory.rs`            | basic HCPs (Reg, Wire, Val, ...)|
| `arena_factory_sp.rs`         | sp_regs (state/sync/cnt/wait)   |
| `arena_factory_ue.rs`         | update events                   |
| `arena_factory_node.rs`       | flow nodes                      |
| `arena_factory_module.rs`     | modules                         |

Convention:
- `make_*` → `is_user_com = false` (system/internal)
- `mk_*`   → `is_user_com = true`  (user-declared)

**Module factories** additionally manage the trace stack:

```rust
// mk_module pushes the module onto the trace stack at the given stage.
pub fn mk_module(&mut self, name: &str, stage: ModuleInitStage) -> ModuleIdent

// mk_top_module always pushes at CompInit (top modules have no parent).
pub fn mk_top_module(&mut self, name: &str) -> ModuleIdent
```

Both also call `stamp_module_to_parent_module`, which sets `master_module_handle`
and `depth_level` on the ident, then writes the stamped ident back into the
`Module` stored in the arena so `module.get_ident()` returns the complete ident.
Callers are responsible for calling `pop_module_trace_stack` when the module's
init stage is complete.

**Critical:** factory methods must read the ident *back from the arena after
insertion* so the returned ident carries the stamped `arena_handle`:

```rust
pub fn add_module(&mut self, m: Module) -> ModuleIdent {
    let h = self.modules.insert(m);
    self.modules.get(h).get_ident()  // <- read AFTER insert
}
```

The older `add_*` HCP methods now follow this pattern. Earlier versions copied
the ident *before* insertion (and so leaked a default `arena_handle`); if you
see that shape anywhere, treat it as a bug to fix.

### 1.5 arena_impl files — per-category CRUD

CRUD on the arena is split by category:

| File                   | Owns                                       |
| ---------------------- | ------------------------------------------ |
| `arena_impl.rs`        | `ModelArena::new` / `reset` + module CRUD  |
| `arena_impl_hwc.rs`    | hardware components (Reg/Wire/.../sp_regs) |
| `arena_impl_ue.rs`     | update events                              |
| `arena_impl_node.rs`   | flow nodes                                 |
| `arena_impl_flow_block.rs` | flow-block primitive CRUD + ONE polymorphic match |
| `arena_impl_flow_block.rs` (second `impl` block) | higher-level flow-block operations (zero match) |

The public surface for each arena type is **only**:

- `add_<thing>(&mut self, T) -> <Thing>Ident` — insert and return the
  handle-stamped ident.
- `take_<thing>(&mut self, ident) -> T` — take the value out (slot stays
  reserved; `T: Default` required).
- `replace_back_<thing>(&mut self, ident, T)` — put it back.

**Do not add typed `get_<thing>(&self) -> &T` or `get_<thing>_mut(&mut self) -> &mut T`
methods on `ModelArena`.** They were deliberately removed. To read or mutate
a stored value, use take/replace_back:

```rust
let mut expr = arena.take_expression(expr_ident);
expr.assign_operand(src, slice);
arena.replace_back_expression(expr_ident, expr);
```

Reasons: (a) take/replace_back makes the borrow of `&mut ModelArena` available
to the inner call; typed `get_mut` does not. (b) it forces a single, uniform
access pattern across all object types.

The one carve-out is internal post-insert ident reads inside
`add_*`/`make_*`/`mk_*` factory methods. Those access the typed `ArenaGroup`
field directly (e.g. `self.regs.get(h).get_ident()`) because it is purely
internal bookkeeping, not a public reading API.

A second narrow carve-out is `pub(crate)` accessors needed by `src/backends/`
code that must read an ident without taking ownership. The only current example
is `get_module_ident_by_handle(h: ArenaHandle) -> ModuleIdent` in `arena_impl.rs`.
Keep these `pub(crate)`, read-only, and few.

### 1.6 Trait-object dispatch (the get-style exception)

Polymorphic accessors that return a borrow of a trait object — e.g.
`get_hcp_assign(&HcpIdent) -> &dyn HcpAssignable`, `get_hcp_readable`,
`get_ncp_node`, `get_ue_common` — are **kept** and live alongside the
take/replace surface (`arena_impl_hwc.rs` for `dispatch_hcp!`,
`arena_impl_node.rs` for `dispatch_ncp!`, `arena_impl_ue.rs` for
`dispatch_ue_common!`).

These cannot be expressed via take/replace_back because the borrow targets a
trait object, not a concrete type. When you add a new HCP/node/UE variant,
add a match arm to the corresponding `dispatch_*!` macro.

#### Flow-block dispatch

Flow blocks (`src/model/flow_block/`) use a specialised dispatch pattern that
**eliminates per-variant match cases** from all higher-level code:

- `arena_impl_flow_block.rs` holds the ONE permitted match inside
  `take_flow_block`. `replace_back_flow_block` needs
  **no match** — it delegates to the trait method:

  ```rust
  pub fn replace_back_flow_block(&mut self, block: Box<dyn FlowBlock>) {
      block.replace_back_into_arena(self);   // each type knows its own slot
  }
  ```

- `FlowBlock` trait (in `flow_block_base.rs`) requires:

  ```rust
  fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena);
  ```

  Every concrete impl (`FlowBlockSeq`, `FlowBlockPar`) calls the matching
  typed `replace_back_flow_block_seq` / `replace_back_flow_block_par`.

- `arena_impl_flow_block.rs` provides all higher-level operations (in its second `impl ModelArena` block)
  (`add_node_to_flow_block`, `add_sub_flow_block_to_flow_block`,
  `build_flow_block`, `summarize_flow_block`). They use take/replace_back
  with `Box<dyn FlowBlock>` — **zero match, zero macro**.

- `FlowBlockIdent` carries `FlowBlockType` **and** `FlowBlockJoinPolicy`
  (both in `flow_block_ident.rs`).
  `FlowBlockType` now has 11 variants: `Sequential`, `Parallel`, `CondIf`,
  `CondElif`, `ZeroCondIf`, `ZeroCondElif`, `ZeroSwitch`, `ZeroSwitchCase`,
  `WhileLoop`, `DoWhile`, `CounterLoop`. They live in three sub-directories:
  `seq/`, `par/`, `cond/` (the six conditional/switch blocks), and `loops/`
  (`FlowBlockWhile`, `FlowBlockDoWhile`, `FlowBlockCounterLoop`).
  `FlowBlockJoinPolicy` is `SubFlow` (a nested child block), `ConFlow` (a
  continuation branch of a conditional chain — elif/else/zelif/zelse), or
  `BasicNodeFlow` (block lowered to a single basic asm node, same entrance as
  `SubFlow`).
  Adding a new block type requires: (1) new variant in `FlowBlockType`,
  (2) new `ArenaGroup` field + CRUD in `arena_impl_flow_block.rs`,
  (3) new arm in the ONE match inside `take_flow_block`,
  (4) `impl FlowBlock` on the new type — nothing else changes.

`NodeWrap` is a by-value summary of node idents, not an owned model object.
Store concrete block types in `ModelArena` and pass `FlowBlockIdent` handles.

#### Flow-block schematics (`flow_block/common/`)

Schematics are reusable node-wiring helpers in `src/model/flow_block/common/`.
They are **not** arena-stored; they live by value inside the matching
`FlowBlock*` struct and own all the logic for wiring that block's node graph.
Each exposes a `build(&mut base, arena) -> NodeWrap`.

| Schematic | Mode enum | Drives |
| --------- | --------- | ------ |
| `SeqSchematic`         | —              | linear sequence of `SequenceEle` (Basic asm-node or SubBlock) |
| `ParSchematic`         | `ParSyncMode` (`AutoSync` / `NoSync`) | parallel fan-out/fan-in |
| `CondSchematic`        | `CondMode`     | conditional / switch branch wiring |
| `CondChain`            | —              | shared elif/else chain bookkeeping for cond blocks |
| `WhileSchematic`       | `LoopMode`     | `while` loop topology (combinational vs sequential) |
| `DoWhileSchematic`     | `LoopMode`     | `do { } while` (body runs at least once) |
| `CounterLoopSchematic` | —              | fixed-count loop |

Re-exported from `flow_block/mod.rs`: `CondMode`, `LoopMode`, `ParSyncMode`.
Use these when implementing new block types that share an existing topology.

`FlowBlockPar` is one unified struct (one `ArenaGroup<FlowBlockPar>`, one
`FlowBlockType::Parallel` variant). The auto-sync / no-sync choice lives
only in `ParSchematic::mode`; callers use `FlowBlockPar::new_auto_sync` or
`FlowBlockPar::new_no_sync`.

### 1.6.1 UE take/replace_back

All four UE types (`UeBasic`, `UeGrp`, `UeCond`, `UeSwitch`) implement `Default`
and have typed `take_ue_*` / `replace_back_ue_*` pairs in `arena_impl_ue.rs`.
A polymorphic `take_ue(ident) -> Box<dyn UpdatingEvent>` (with ONE match) and
`replace_back_ue(Box<dyn UpdatingEvent>)` (zero match, delegates to
`replace_back_into_arena`) are also present for callers that do not need a
concrete type.

### 1.7 Grouped index arrays

When a struct holds multiple categorized lists of the same handle type, prefer
`[Vec<HcpIdent>; HwComponentType::COUNT]` indexed by the type enum, with a
single `add_*` that reads the type off the ident. See `Module::sp_regs` /
`Module::user_hw` for the canonical example. Avoid one field per HW type.

### 1.8 Clock-signal policy

Source of truth: `src/model/controller/clock_mode.rs` (top-of-file block).
Summary of how a clock source gets onto an `UpdateEvent`:

- **`ClockMode::ClkFree`** — no clk wiring. The event lives combinationally;
  `UeBasic::clk_signal_i` stays `None` and Verilog emits an `always @(*)`.

- **`ClockMode::PosEdge` / `NegEdge`** — clk must be a concrete signal:

  1. **User-level assignment** — `AssignMeta` and its `UpdateEvent` are built
     with `clk_signal = None`.  The clk is filled in during the enclosing
     flow block's build phase via `FlowBlockBase::build_common_hw`, drawn
     from the block's `ext_trigger_node.clk_node_i` (forwarded through
     `init_node_trigger_for_basic_node` → `set_clk_src_for_basic_node`).

  2. **Flow-block internal registers** (no `AssignMeta`, only `UpdateEvent`)
     — the clk is wired when the owning node asks the register to build its
     update event, again sourced from that node's `NodeTrigger::clk_node_i`.

`ExtSigType::Clk` is the single-source variant in `FlowBlockBase` (asserted by
`gen_clk_node` — no OR fan-in like the other ExtSigType slots).  Each flow
block carries exactly one Clk source per `ext_signals` slot.

---

## 2. Memory management mechanism

### 2.1 Ownership rules

- **One owner per object: the arena.** No object stores another model object
  by value or by `Box`. Cross-references are always `*Ident` (Copy).
- **No `Rc` / `Arc` / `RefCell` for model state.** Mutability is mediated by
  `&mut ModelArena`.
- **No raw pointers.** The C++ reference uses raw pointers heavily; do not
  port that. Translate `T*` to `TIdent`.

### 2.2 Lifetime / re-borrow pattern

When a method needs `&mut self` *and* `&mut ModelArena` (e.g. building update
events while mutating the owning reg), use take/replace_back:

```rust
let mut reg = arena.take_reg(reg_ident);
reg.build_update_event(arena);          // arena is freely usable here
arena.replace_back_reg(reg_ident, reg);
```

`ArenaNode::is_temp_taken` debug-asserts against double-take.

### 2.3 Generation safety

`ArenaHandle` carries a `generation: u32`. `ArenaGroup::free` bumps the slot's
generation, so any stale handle to a freed slot fails the `get`/`get_mut`
generation assert. Reuse via `free_slots` is automatic.

### 2.4 Reset

`ModelArena::reset` re-initialises every `ArenaGroup`. Anything new added to
the arena must be added to **both** `ModelArena::new` and `reset`.

### 2.5 Global ID

`IdentBase::global_id` comes from a process-wide `AtomicU64` (`GLOBAL_MODEL_ID`
in `identifier.rs`). It increments forever and never resets across `reset()`.
Treat it as opaque; do not depend on its absolute value.

---

## 3. Naming conventions

### 3.1 Files and modules

- `snake_case.rs` for files, matching the primary type they define
  (`module.rs` defines `Module`).
- One primary type per file; small companion types (enums, helper structs) may
  live alongside.
- Submodule directories use `mod.rs` that only re-exports — no logic.

### 3.2 Types

- `PascalCase` for structs, enums, traits.
- Ident types end with `Ident` (`HcpIdent`, `ModuleIdent`).
- Trait-object-suitable traits end with `-able` (`HcpAssignable`,
  `HcpReadable`, `HcpAccessible`).
- Enum variants: `PascalCase`. Avoid C-style `SCREAMING_PREFIX_VARIANT`.

### 3.2.1 Ident variable naming — `_i` suffix

Any **variable or field** whose type is an `*Ident` handle (e.g. `NcpIdent`,
`HcpIdent`, `FlowBlockIdent`, `ModuleIdent`, …) must carry an `_i` suffix so
it is immediately clear it is a lightweight handle, not the object itself:

```rust
let state_i    = arena.make_state_node(...);   // NcpIdent
let asm_node_i = ...;                          // NcpIdent
self.sync_reg_i: HcpIdent;                     // struct field
```

`seq_schematic.rs` and `par_schematic.rs` have been updated to follow this
rule. Other files still use bare names (`state`, `asm`, `syn`, …) — fix them
as you touch those files; do not do a mass rename in a single PR.

**No single-letter handle names.** Even for a short-lived local inside a
take/replace_back block, do not name an ident `h` / `r` / `s`. Give it the same
descriptive `_i` name it would have at the use site (e.g. `user_hold_i`,
`user_reset_i`), so the value reads the same whether it is bound in a scratch
block or returned. Single letters are only acceptable for the taken object
itself (e.g. `let arb = arena.take_arb(...)`), never for the handles read off it.

### 3.3 Functions

- `snake_case`.
- Constructor pair: `new(is_user_com: bool, name, ...)` is the full
  constructor; `mk(name, ...)` is the user-declared shorthand
  (`is_user_com = true`).
- Arena factories: `make_<thing>` (system) and `mk_<thing>` (user).
- Getters: `get_<field>` / `get_<field>_mut`. No bare-field getters.
- Adders: `add_<thing>` (push into a Vec/arena). Plural collection getter:
  `get_<things>` returning `&Vec<…>`.

### 3.4 Constants

- `SCREAMING_SNAKE_CASE`.
- Update-event priorities live as `DEFAULT_UE_PRI_*` consts at module top.

### 3.5 Identifier prefixes (in `build_unique_name`)

`HwComponentType::global_prefix` defines the canonical short prefix for each
HW type (`REG`, `WIRE`, `SR_ST`, `MODULE`, ...). Use those when constructing
unique names — do not invent new ones.

---

## 4. Other things that are relevant

### 4.1 Pre-existing build errors

The current baseline is **0 errors**. `cargo build` should complete cleanly
(warnings are acceptable). The Python build, `cargo build --features python`,
must also stay at 0 errors. Any new error you introduce is yours to fix.

### 4.2 What is *not yet* ported from C++

Do not assume these exist on the Rust side; if you need them, check first:

- full controller-driven flow-block construction (`buildAll` / `buildFlow`)
- `Box`, `nest`, `PmVal`, `ModelInterface`
- `ModuleSimEngine`, `ModuleGen`, `ModelDebugger`
- `controller` (`ctrl->on_module_init_components` etc.) — only `clock_mode`
  is ported

When porting, scope the new struct to fields whose dependencies *do* exist.

### 4.3 Workflow expectations

- Run `cargo build` after non-trivial changes. The baseline is 0 errors
  (§4.1); any new error is yours.
- Do not run `git push`, `cargo publish`, or anything that mutates remote
  state without explicit user approval.
- Default to editing existing files; do not add documentation files unless
  asked. This file is the exception.

### 4.4 C++ reference

`/media/tanawin/tanawin1701e/project2/Kathryn` is the source of truth for
*semantics* (what a thing should do). It is **not** the source of truth for
*structure* (how to organise it in Rust). When in doubt:

1. Read the C++ header to understand the contract.
2. Find the closest Rust analogue already in this repo.
3. Match the existing Rust pattern, even if it differs from C++.

### 4.5 Memory notes

The user maintains personal cross-session memory under
`~/.claude/projects/.../memory/`. Project-scoped facts that belong in the repo
itself go in this file (CLAUDE.md). When adding a new pattern that future
contributors must follow, update CLAUDE.md — do not rely on agent memory.

---

## 5. Backends (`src/backends/`)

Code that consumes the model but does not mutate the core data model lives under
`src/backends/`. It is a separate top-level module declared in `main.rs`.

### 5.1 Module tree

```
src/backends/
  mod.rs              — re-exports sub-modules only
  common/
    mod.rs            — re-exports sub-modules only
    graph.rs          — DfsModuleIter + module-ancestor path utilities
    io_op.rs          — IoWire construction and reuse helpers
    routing.rs        — cross-module IO routing (route_and_remap_io_model and friends)
  verilog/
    mod.rs            — re-exports sub-modules only
    arena_ext_vb.rs   — impl ModelArena { take_hcp_vb, take_ue_vb } (ONE match each)
    hw_component/
      mod.rs          — re-exports sub-modules only
      util_vb.rs       — signal_width, slice_to_verilog, sensitivity_list, fmt_operand, gen_procedure_blk helpers
      reg_vb.rs        — HcpBaseVb for Reg + all sp_reg types (macro impl_reg_vb!)
      wire_vb.rs       — HcpBaseVb for Wire (always combinational)
      io_wire_vb.rs    — HcpBaseVb for IoWire
      mem_blk_vb.rs    — HcpBaseVb for MemBlk
      mem_ele_vb.rs    — HcpBaseVb for MemEle
      val_vb.rs        — HcpBaseVb for Val
      expression_vb.rs — HcpBaseVb for Expression
      common/
        mod.rs        — re-exports sub-modules only
        hcp_base_vb.rs      — HcpBaseVb trait (declaration + behaviour phases)
        update_event_vb.rs  — VerilogUpdateEvent trait + all 4 impls + transpile_ue
```

### 5.2 `graph.rs` — module iterator and ancestor utilities

```rust
// Lazy pre-order DFS iterator over the module subtree.  Arena borrow is held
// only during each next_module call, leaving it free for callers between steps.
pub struct DfsModuleIter { ... }
impl DfsModuleIter {
    pub fn new(root_i: ModuleIdent) -> Self;
    pub fn next_module(&mut self, arena: &mut ModelArena) -> Option<ModuleIdent>;
}

// Returns (path_a, path_b): each Vec walks from the given module up to the
// lowest common ancestor (inclusive).  Uses depth_level to balance the walk.
// Takes &ModelArena (immutable — no mutation needed).
pub fn find_common_ancestor_module_paths(arena: &ModelArena, a: ModuleIdent, b: ModuleIdent)
    -> (Vec<ModuleIdent>, Vec<ModuleIdent>)

// Convenience: resolves master_module_i on each HcpIdent, then delegates above.
pub fn find_common_ancestor_module_paths_from_hcp(arena: &ModelArena, a: HcpIdent, b: HcpIdent)
    -> (Vec<ModuleIdent>, Vec<ModuleIdent>)
```

### 5.3 `io_op.rs` — IoWire helpers

```rust
// Create an IoWire in src_module.
// actual_src_signal — the original signal the wire represents (used for naming + reuse lookup).
// agent_src_signal  — the wire that actually drives this IO at this level (may equal actual
//                     at the source level, or be the previous level's IO wire on the input side).
// Pushes src_module onto the trace stack at CompInit, pops on return.
pub fn build_io_wire(
    arena            : &mut ModelArena,
    src_module       : ModuleIdent,
    actual_src_signal: HcpIdent,
    agent_src_signal : HcpIdent,
    is_input         : bool,
) -> HcpIdent

// Search src_module's registered IoWires (user + internal) for one that already
// binds actual_src_signal in the requested direction.  Returns Some(ident) or None.
pub fn find_reusable_io_wire(
    arena            : &mut ModelArena,
    module           : ModuleIdent,
    actual_src_signal: HcpIdent,
    is_input         : bool,
) -> Option<HcpIdent>
```

`IoWire` stores both `actual_src_signal_i` (the origin) and `agent_src_signal_i` (the
immediate driver at this level). `find_reusable_io_wire` matches on `actual_src_signal_i`
and `is_input` so the reuse check is always anchored to the original signal regardless
of how many IO hops exist between the source and this module.

### 5.4 Verilog backend — `HcpBaseVb` trait

Every HCP type that the Verilog emitter handles implements `HcpBaseVb`
(`src/backends/verilog/hw_component/common/hcp_base_vb.rs`):

```rust
// ---- required — no default ----
fn gen_type_vb               (&self) -> String;   // e.g. "reg [7:0]" or "wire"
fn gen_var_name_vb           (&self) -> String;   // signal name in emitted Verilog
fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena);  // arena round-trip

// ---- count queries — default 0; override only when non-zero ----
fn amt_io_line_vb      (&self) -> u32 { 0 }        // IO port declaration lines
fn amt_init_line_vb    (&self) -> u32 { 0 }        // initial-block lines
fn amt_precedure_blk_vb(&self) -> u32 { 0 }        // always-block count

// ---- generation — default panics; implement when count > 0 ----
fn gen_io_line_vb      (&self, idx: u32, arena: &mut ModelArena, fw: &mut FileWriter);
fn gen_init_line_vb    (&self, idx: u32, arena: &mut ModelArena, fw: &mut FileWriter);
fn gen_procedure_blk_vb(&self, idx: u32, arena: &mut ModelArena, fw: &mut FileWriter);
```

`gen_procedure_blk_vb` calls `transpile_ue` for each event in the HCP's `UpdatePool`
and wraps the result in `always @(sensitivity) begin … end`.
`replace_back_into_arena_vb` mirrors the `FlowBlock` pattern — each concrete type
puts itself back into the correct typed arena slot; callers use
`arena.replace_back_hcp_vb(v)` with zero match.

Shared string helpers live in `util_vb.rs`:

| Function | Returns |
|----------|---------|
| `signal_width(size: i32)` | `"[N-1:0] "` for multi-bit, `""` for 1-bit |
| `slice_to_verilog(s: &Slice)` | `"[stop-1:start]"` or `""` for default `{-1,-1}` |
| `sensitivity_list(clk_mode)` | `"posedge clk"`, `"negedge clk"`, or `"*"` |
| `fmt_operand(opr, slice, arena, active_i, active_name)` | `"var_name[slice]"` with self-ref guard |
| `gen_procedure_blk(hcp, active_i, arena, fw)` | write-through always-block helper for clocked HCPs |

### 5.5 Verilog backend — scalable dispatch in `arena_ext_vb.rs`

`arena_ext_vb.rs` holds **two** single-match dispatch functions, one per trait:

```rust
// src/backends/verilog/arena_ext_vb.rs
impl ModelArena {

    /// ONE match for HCP → HcpBaseVb.  Add one arm when a new HwComponentType is added;
    /// nothing else in the Verilog emitter changes.
    pub fn take_hcp_vb(&mut self, ident: HcpIdent) -> Box<dyn HcpBaseVb> {
        match ident.get_hw_type() { ... }
    }

    /// Zero match — each type's replace_back_into_arena_vb knows its own slot.
    pub fn replace_back_hcp_vb(&mut self, v: Box<dyn HcpBaseVb>) { v.replace_back_into_arena_vb(self); }

    /// ONE match for UE → VerilogUpdateEvent.  Add one arm when a new UeType is added.
    pub fn take_ue_vb(&mut self, ident: UpdateEventIdent) -> Box<dyn VerilogUpdateEvent> {
        match ident.get_ue_type() { ... }
    }
}
```

The public free function `transpile_ue` (in `update_event_vb.rs`) has **zero match**:

```rust
pub fn transpile_ue(ue_i, op_templates, front_space, arena) -> String {
    let ue = arena.take_ue_vb(ue_i);
    let s  = ue.transpile(op_templates, front_space, arena);
    ue.replace_back_into_arena_vb(arena);    // trait dispatch, no match
    s
}
```

`VerilogUpdateEvent` requires two methods on every concrete UE type:

```rust
pub trait VerilogUpdateEvent {
    fn transpile(&self, op_templates: Vec<String>, front_space: u32, arena: &mut ModelArena) -> String;
    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena);
}
```

**Adding a new HCP type** only touches:
1. New `HwComponentType` variant.
2. One new arm in `take_hcp_vb` (`arena_ext_vb.rs`).
3. `impl HcpBaseVb for NewHcpType` in the matching `*_vb.rs` file.

`take_ue_vb`, `transpile_ue`, `gen_procedure_blk_vb`, and all other HCP impls stay **untouched**.

**Adding a new UE type** only touches:
1. New `UeType` variant.
2. One new arm in `take_ue` (`arena_impl_ue.rs`) — unavoidable.
3. One new arm in `take_ue_vb` (`arena_ext_vb.rs`).
4. `impl VerilogUpdateEvent for NewUeType` in `update_event_vb.rs`.

`transpile_ue`, `gen_procedure_blk_vb`, and all HCP impls stay **untouched**.

**`op_templates` contract:** the caller (HCP's `gen_procedure_blk_vb`) builds a
template string containing the destination name with placeholders:

```
"signal_name{DES_SLICE} <= {SRC_VAL}{SRC_SLICE};"
```

`UeBasic::transpile` substitutes `{DES_SLICE}`, `{SRC_VAL}`, `{SRC_SLICE}` using
`slice_to_verilog` and `srci.get_global_name()`. Container UEs (`UeGrp`, `UeCond`,
`UeSwitch`) pass `op_templates` unchanged to recursive `transpile_ue` calls.

**Backend `impl ModelArena` blocks:** backends may add methods to `ModelArena` in
their own files (e.g. `arena_ext_vb.rs`) as additional `impl ModelArena` blocks.
These must use only the already-public typed `take_*` / `replace_back_*` methods —
not the `pub(super)` `ArenaGroup` fields. Keep such extensions in the relevant
backend sub-directory.

### 5.6 `routing.rs` — cross-module IO routing

`routing.rs` lives in `src/backends/common/` and automates the creation of IoWire
chains that thread a signal across the module hierarchy.

```rust
// Internal: create IoWires along the output (source→LCA) and input (LCA→dest) sides.
// input_paths and output_paths come from find_common_ancestor_module_paths;
// the last element of each is the LCA and is skipped (it is the crossing point).
fn route_io_base(
    input_paths    : &mut Vec<ModuleIdent>,
    output_paths   : &mut Vec<ModuleIdent>,
    actual_src_i   : HcpIdent,
    model_arena    : &mut ModelArena,
) -> HcpIdent   // returns the IoWire ident visible in the destination module

// Route actual_src_i into des_mod_i across the module tree; returns the IoWire ident
// that des_mod_i should use in place of the original cross-module reference.
fn route_io_hw_comp(actual_src_i: HcpIdent, des_mod_i: ModuleIdent, model_arena: &mut ModelArena) -> HcpIdent

// Walk every HCP in module_i, find cross-module deps, route IO chains, then rewrite
// all dep handles inside the module to point at the new IoWires.
fn route_and_remap_io_module(module_i: ModuleIdent, model_arena: &mut ModelArena)

// Top-level entry point: DFS over the full module tree and call route_and_remap_io_module
// on each module.  Requires a top module to be set on the arena.
fn route_and_remap_io_model(model_arena: &mut ModelArena)
```

The routing pipeline: `gather_dep_hcps` collects every HCP referenced by a module's
HW components; cross-module deps are routed via `route_io_hw_comp`; `remap_dep_hcps`
rewrites all handles using the resulting `old → IoWire` map.

`find_reusable_io_wire` is consulted before `build_io_wire` at every hop so duplicate
IO wires are never created for the same `(actual_src_signal, direction)` pair.

---

## 6. Code style

All generated and edited code must follow the owner's personal formatting style
defined in `~/.claude/skills/codestyle-skill/SKILL.md`. Key rules:

- **Column-align** `:` in struct fields and function parameters so types form a vertical column.
- **Collapse trivial getters** to a single line; align return types across the group.
- **`---- Section ----` separator comments** to divide logical groups inside `impl` blocks or files.
- **Align match/switch arms** so `=>` or `:` lines up vertically.
- **Multi-line signatures** when 3+ parameters: one param per line, closing delimiter on its own line, trailing comma.
- **`_i` suffix** on any variable or field whose type is a `*Ident` handle.
- **Brief comments** — one sentence, explain why not what; no multi-paragraph docstrings.

---

## 7. Python bindings (`src/applications/py/`)

The Python frontend (PyO3 + maturin) is the only place in the crate that
contains PyO3 macros. It is `#[cfg(feature = "python")]`-gated, so the default
build never compiles it. This `src/applications/py/` layer is the **low-level**
binding (`PyModelArena` + ident wrappers); a separate **pure-Python DSL** wraps
it in `py/kathryn/` (see §7.5).

Packaging is a maturin **mixed project** (`pyproject.toml`): `python-source = "py"`,
`module-name = "kathryn._kathryn"`, `features = ["python"]`. The native extension
is built as `kathryn._kathryn` and dropped inside the pure-Python package
`py/kathryn/`, whose `__init__.py` re-exports it. Users `import kathryn`.
The `#[pymodule]` fn is therefore named `_kathryn` (its name must match the last
`module-name` component).

### 7.1 Golden rule — the core stays PyO3-free

Core model types (`HcpIdent`, `Slice`, `FlowBlockIdent`, `ModelArena`, …) carry
**no** PyO3 macros. For each one exposed to Python there is a thin newtype
wrapper under `py/` (`PyHcpIdent`, `PySlice`, `PyFlowBlockIdent`,
`PyModelArena`) that is the *only* Python-visible face. Wrappers hold the core
value in a `pub(crate) inner` field and provide `From<Core> for PyWrapper` **and**
`From<PyWrapper> for Core` so factory bodies read
`self.arena.make_x(...).into()` and accept `arg.into()`. Ident/value wrappers
are `#[pyclass(..., from_py_object)]` + `#[derive(Clone, Copy)]` so they pass
by value as arguments.

### 7.2 Module tree mirrors `src/model/`

```
src/applications/py/
  mod.rs                      — #[pymodule] _kathryn(); registers classes + LogicOp enum
  model/
    mod.rs
    model_arena.rs            — #[pyclass(unsendable)] PyModelArena { arena: ModelArena }
    arena_factory_hwc_py.rs        — mk_reg / mk_wire / mk_io_wire / mk_val / mk_mem_blk / mk_mem_ele
    arena_factory_hwc_expr_py.rs   — mk_expression (binary) / mk_expression_single (unary ~,!) / mk_extend_bit
    arena_impl_hwc_py.rs           — gen_basic_assign and other higher-level HWC ops
    arena_factory_flow_block_py.rs — mk_flow_block_* (seq/par/cond/zero/while/do_while/counter)
    arena_impl_flow_block_py.rs    — initialize_flow_block / finalize_flow_block
    arena_factory_module_py.rs     — mk_module (sub-module; top is made in PyModelArena::new)
    arena_impl_module_py.rs        — initialize_module / finalize_module
    hw_component/common/{hcp_ident_py.rs, slice_py.rs} — PyHcpIdent, PySlice
    flow_block/flow_block_ident_py.rs              — PyFlowBlockIdent
    module/module_ident_py.rs                      — PyModuleIdent
```

The file split mirrors the host `arena_factory_*` / `arena_impl_*` split. PyO3's
`multiple-pymethods` feature lets every file add its own `#[pymethods] impl
PyModelArena` block. Add a new Python file under the mirror path that matches
the host file you are wrapping; do not pile everything into one block.

### 7.3 Conventions

- **`PyModelArena` is `unsendable`** — single-threaded by design, mirroring the
  arena being the sole Rust-side owner. Its `#[new]` pushes a top module up
  front (`mk_top_module`) so component factories always have a module on the
  trace stack to attach to.
- **Every wrapper method is user-declared**: HWC factories pass
  `is_user_com = true` (i.e. they wrap the host `make_*`/`mk_*` user path).
- **`_py` file suffix** distinguishes the Python mirror from the host file.
- **Naming**: host `make_x` → Python `mk_x` (Python is always the user surface).
- **Flow-block lifecycle** is driven explicitly from Python:
  `initialize_flow_block` (push onto the init stack) → build contents →
  `finalize_flow_block` (pop + attach to parent/module). The construction phase
  stops at finalize; there is no Python-side hardware-build hook (see §7.5).
- **Module lifecycle** mirrors the flow-block one against the *module trace
  stack*: `mk_module` (create + stamp parent) → `initialize_module` (push so
  components/flow-blocks attach) → build contents → `finalize_module` (pop +
  register with the enclosing module via `add_user_sub_module`).
  `PyModelArena::new` opens the top module up front. The entire Python
  construction phase runs the active module at `ModuleInitStage::FlowBlockInit`
  (both HW declaration and top-flow-block finalize work there; the C++
  component/flow two-phase split is unused while controller hooks are unported).
  `finalize_module` **must** call `add_user_sub_module` — `mk_module` only
  stamps the parent handle, so without it the build DFS never descends into the
  sub-module (a known host gotcha).

### 7.4 Enums — single source of truth

`LogicOp` is exposed as a Python `IntEnum` built at module-init by walking the
core `LogicOp::from_index` / `variant_name` (see `add_logic_op_enum` in
`mod.rs`). Python never hardcodes op ints, and `mk_expression` decodes the int
back through the *same* `from_index`, so the two sides cannot drift. Follow this
pattern for any new enum crossing the boundary — never duplicate variant lists
in Python. `mk_expression` is binary-only — it rejects single-operand ops
(`is_single_opr`: `~`, `!`, `Assign`) plus `ExtendBit` / `Dummy`; unary `~`/`!`
go through `mk_expression_single`, bit-extend through `mk_extend_bit`, assignment
through `gen_basic_assign`.

### 7.5 Pure-Python DSL (`py/kathryn/`)

A thin pure-Python package wraps the low-level binding into an HDL-like DSL.
Every wrapper holds **only** the Rust ident (ownership stays in Rust); every
operation routes through one process-wide `ModelArena`.

- `_session.py` — builds the singleton **empty** `ModelArena` **at import**
  (`sys.modules` caching makes repeated `import kathryn` reuse it). **The top
  module is explicit**: no top is created at import or by `reset()`; the user
  builds a `Module` and registers it via `set_top(module)` (public entry in
  `module.py`, delegating to the private `_session._set_top`), which calls
  `set_top_module` only — it records the top ident but does **not** re-open a
  module scope. All hardware/flow must be declared inside the top `Module`'s own
  `@init` / `@flow` methods; a bare top-level declaration outside a class finds no
  open module on the trace stack and **panics by design**. Also holds the
  `arena()` accessor, `reset()`, the per-prefix auto-name counter, `gen_flow()`,
  and `build_flow()`.
- `signal.py` — `SignalRef` (ident + optional `Slice`) with operator overloading:
  binary `+ - * / % & | ^ << >> < <= > >= == !=` → `mk_expression`; unary `~` →
  `mk_expression_single`; `.land/.lor/.lnot/.slt/.sgt/.extend` for ops with no
  Python operator. `==`/`!=` return an expr (not bool), so `__hash__ = None`.
  **Inclusive slicing**: `sig[hi, lo]` → `Slice(lo, hi+1)`, `sig[i]` → single bit.
  Assignment via augmented ops: `|=` (clocked: reg/mem) and `*=` (comb:
  wire/io_wire), both → `gen_basic_assign` with a destination-type guard; sliced
  assignment (`a[7,0] |= b`) works via a `_ASSIGNED` sentinel + `__setitem__`.
  `expr(SignalRef)` is the result type (not assignable).
- `hw_component.py` — lowercase classes `reg/wire/val/io_wire/mem_blk/mem_ele`;
  `name` optional+last (auto-named when omitted).
- `flow_block.py` — flow blocks are context managers: `with seq(): …`,
  `with sif(cond): …`. `__enter__` opens the scope (initialize), `__exit__`
  finalizes. There is no block-build hook (see below).
- `module.py` — modules use a **class form**: extend `Module` and decorate methods
  with `@init` (hardware declaration) and `@flow` (flow-block construction).
  Construction is **two-phase**:
  - `@init` runs **eagerly** in `__init__`, once, inside a single
    `track_module_at_com_init`/`untrack_module_at_com_init` scope, so
    `self.x = reg(...)` is declared into the module up front.
  - `@flow` is **deferred** (lazy): the bound flow methods are not called at
    construction; each registers into ONE process-wide pool
    (`_session.flow_pool`, keyed by module ident) via `_session.register_flow`.
    The top-level `_session.gen_flow()` (exported from the package) builds
    **every** module's flows from that single pool — there is no per-instance
    build. Each flow call **re-opens its own module scope** (its own
    `track_module_at_flow_init`/`untrack_module_at_flow_init` pair), so those two
    calls fire many times across modules. The pool is non-consuming, so
    `gen_flow()` is safely re-runnable; `_session.reset()` clears it.

  Phase methods run in definition order, base classes first
  (`Module._phase_methods` walks the reversed MRO), so an inherited `@init` runs
  before a derived one; each name runs once via the most-derived binding. The
  decorators don't wrap the method — they tag the function with `_kathryn_phase`.
  (There is no context-manager `module()` form — modules are always classes.)
- `__init__.py` — defines `__all__` so `from kathryn import *` exposes the whole DSL.

**Operands must be signals** — int literals aren't auto-wrapped (`a >> val(8,2)`,
not `a >> 2`). **Conditional blocks hold sub-blocks, not direct nodes** — put a
`with seq():` inside an `if`/`while` body (a model constraint).

**Construct then build.** The full pipeline is now:

```python
set_top(Top())   # register the user's top Module
gen_flow()       # construct every module's deferred @flow blocks (re-runnable)
build_flow()     # host build pass over the whole module tree (run once)
```

`build_flow()` (`_session.build_flow` → `PyModelArena::build_flow` →
`ModelArena::build_flow`) starts from the top module: `build_flow_as_top_module`
creates the primitive `clk` / master-reset (`mrst`) input wires and the start
node itself (it **asserts the top start node is unset** beforehand), then
forwards clk + master-reset down to every sub-module. It is **not re-runnable**
(the second call would hit the fresh-start-node assert). The flow-block-level
`build_flow_block` binding is still intentionally absent; the build is driven at
the model/top level, not per block. `__exit__` on a flow-block context manager
still only finalizes — there is no `ctx.build()`.

Tests: `py/tests/test_smoke.py` (run `PYTHONPATH=py pytest py/tests` after the
extension is built). Without maturin, `cargo build --features python` then copy
`target/debug/libkathryn.so` → `py/kathryn/_kathryn.so` (the `PyInit__kathryn`
symbol makes it importable as `kathryn._kathryn`).

**Gotcha — copy to the ABI-tagged name, not just `_kathryn.so`.** If a previous
maturin/`pip install` left an ABI-tagged extension in the package (e.g.
`py/kathryn/_kathryn.cpython-313-x86_64-linux-gnu.so`), CPython loads *that* in
preference to the bare `_kathryn.so`, so copying only to `_kathryn.so` silently
runs the **stale** build. Overwrite the ABI-tagged file too (copy to both):

```sh
cargo build --features python
cp target/debug/libkathryn.so py/kathryn/_kathryn.cpython-313-x86_64-linux-gnu.so   # the one actually loaded
cp target/debug/libkathryn.so py/kathryn/_kathryn.so                                # fallback name
```

To see which file is actually imported: `PYTHONPATH=py python3 -c "import
kathryn._kathryn as k; print(k.__file__)"`.

**Model fix made for the DSL:** `HcpAssignable::gen_update_event`
(`hcp_assign.rs`) now resolves a default/invalid `src_slice` to the source's full
slice via `arena.get_hw_slice(&srci)` — mirroring how `des_slice` falls back to
`get_des_slice()`. Previously a full-width source (`Slice::default()` = `(-1,-1)`)
hit the `get_match_size_sub_slice` validity assert.

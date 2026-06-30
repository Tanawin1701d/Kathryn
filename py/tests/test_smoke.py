# Smoke tests for the Kathryn Python DSL. Each test resets the singleton arena
# first for isolation. Run with: pytest py/tests  (after `maturin develop`).

import os
import tempfile

import pytest
import kathryn as k
from kathryn import (
    reset, set_top, reg, wire, val, mem_blk, mem_ele,
    seq, sif, par, expr,
    pick, pif, pidef,
    Module, init, flow, gen_flow, build_flow, emit_verilog,
    priority, set_priority, set_priority_auto, get_priority, get_priority_mode,
    DEFAULT_UE_PRI_USER, DEFAULT_UE_PRI_RST, DEFAULT_UE_PRI_MIN,
    Karray, kaf, HwComponentType, oh, Reduce,
)


def setup_function(_):
    reset()
    set_top(Module("top"))   # top is explicit — user provides the Module to set as top


def test_singleton_survives_double_import():
    import kathryn as a
    import kathryn as b
    assert a._session.arena() is b._session.arena()


def test_hw_constructors_and_types():
    # Hardware must be declared inside a module scope; @init opens/closes one
    # eagerly, so capture the refs into a dict and assert on them afterwards.
    h = {}

    class worker(Module):
        @init
        def decl(self):
            h["reg"]  = reg(8)
            h["wire"] = wire(8)
            h["val"]  = val(8, 3)
            h["blk"]  = mem_blk(8, 4)
            h["ele"]  = mem_ele(h["blk"], val(4, 0), 8, True)   # read element
            iow = wire(1)
            iow.mark_input("test_in")
            h["iow"] = iow

    worker()
    assert h["reg"].hw_type  == "REG"
    assert h["wire"].hw_type == "WIRE"
    assert h["val"].hw_type  == "VAL"
    assert h["blk"].hw_type  == "MEM_BLOCK"
    assert h["ele"].global_id > 0
    assert h["iow"].is_io


def test_optional_names_autogenerate():
    h = {}

    class worker(Module):
        @init
        def decl(self):
            h["a"] = reg(8)              # no name -> auto
            h["b"] = reg(8, "explicit")

    worker()
    assert h["a"].global_id != h["b"].global_id


def test_binary_operator_builds_expression():
    h = {}

    class worker(Module):
        @init
        def decl(self):
            a, b = reg(8), wire(8)
            h["e"]   = a + b
            sh       = val(8, 2)            # operands must be signals, not int literals
            h["shr"] = (a & b) >> sh        # chaining + bitwise + shift
            h["eq"]  = a == b               # relational

    worker()
    assert isinstance(h["e"], expr)
    assert h["e"].hw_type   == "EXPR"
    assert h["shr"].hw_type == "EXPR"
    assert h["eq"].hw_type  == "EXPR"


def test_unary_invert_builds_expression():
    h = {}

    class worker(Module):
        @init
        def decl(self):
            a = reg(8)
            h["inv"]  = ~a
            h["lnot"] = a.lnot()

    worker()
    assert h["inv"].hw_type  == "EXPR"
    assert h["lnot"].hw_type == "EXPR"


def test_inclusive_slicing():
    h = {}

    class worker(Module):
        @init
        def decl(self):
            h["a"] = reg(16)

    worker()
    a = h["a"]
    s = a[8, 0]._slice          # bits 8..0 inclusive
    assert (s.start, s.stop) == (0, 9)
    one = a[3]._slice           # single bit
    assert (one.start, one.stop) == (3, 4)
    with pytest.raises(TypeError):
        a[8:0]                  # python-slice form is rejected


def test_clocked_assignment_with_ior():
    class worker(Module):
        @flow
        def f(self):
            a, b, c = reg(8), wire(8), reg(8)
            with seq():
                c |= a + b              # reg <- expr (clocked)

    worker()
    gen_flow()


def test_comb_assignment_with_imul():
    class worker(Module):
        @flow
        def f(self):
            a, b = reg(8), wire(8)
            with seq():
                b *= a                  # wire <- reg (combinational)

    worker()
    gen_flow()


def test_assignment_operator_guards():
    # The guard raises in Python before any arena assign; signals still need a
    # scope to be created, so declare them in @init and check the guards after.
    h = {}

    class worker(Module):
        @init
        def decl(self):
            h["r"], h["w"] = reg(8), wire(8)

    worker()
    with pytest.raises(TypeError):
        h["w"] |= h["r"]                # |= on a wire is wrong
    with pytest.raises(TypeError):
        h["r"] *= h["w"]                # *= on a reg is wrong


def test_sliced_assignment():
    class worker(Module):
        @flow
        def f(self):
            a, b = reg(16), wire(16)
            with seq():
                a[7, 0] |= b[7, 0]      # sliced clocked assign
                b[15, 8] *= a[15, 8]    # sliced comb assign

    worker()
    gen_flow()


def test_nested_module_and_flow_scopes():
    class sub(Module):
        @flow
        def f(self):
            a, b, c = reg(8), wire(8), reg(8)
            with seq():
                c |= a + b
            with sif(a == b):
                with seq():        # cond blocks hold sub-blocks, not direct nodes
                    b *= a

    sub()
    gen_flow()


def test_class_based_module_runs_init_eager_flow_deferred():
    order = []

    class my_module(Module):
        @init
        def my_init(self):
            order.append("init")
            self.x = reg(5)         # hardware declared into this module's scope

        @flow
        def my_flow(self):
            order.append("flow")
            with seq():             # flow block attached to this module's scope
                self.x |= 0         # a seq must hold at least one node

    m = my_module()
    assert order == ["init"]                   # @init eager; @flow deferred
    assert m.ident.global_id > 0
    assert m.x.hw_type == "REG"
    gen_flow()                               # global build drains the pool
    assert order == ["init", "flow"]
    gen_flow()                               # pool is non-consuming, re-runnable
    assert order == ["init", "flow", "flow"]


def test_gen_flow_builds_all_modules_from_one_pool():
    order = []

    class mod_a(Module):
        @flow
        def f(self):
            order.append("a")
            r = reg(1)
            with seq():
                r |= 0          # a seq must hold at least one node

    class mod_b(Module):
        @flow
        def f(self):
            order.append("b")
            r = reg(1)
            with seq():
                r |= 0

    mod_a(); mod_b()
    assert order == []                         # both deferred into the one pool
    gen_flow()                               # single build covers every module
    assert order == ["a", "b"]


def test_class_module_phase_inheritance_runs_base_first():
    order = []

    class base(Module):
        @init
        def base_init(self):
            order.append("base")

    class derived(base):
        @init
        def derived_init(self):
            order.append("derived")

    derived()
    assert order == ["base", "derived"]       # inherited @init runs before derived


def test_asm_priority_constants_exposed():
    # The DEFAULT_UE_PRI_* constants are sourced straight from the host consts.
    assert DEFAULT_UE_PRI_USER == 10
    assert DEFAULT_UE_PRI_RST  == 2147483647
    assert DEFAULT_UE_PRI_MIN  == 0


def test_asm_priority_get_set_and_scope():
    # Default mode is Auto at the host's user priority.
    set_priority_auto()
    assert get_priority_mode() == "Auto"
    assert get_priority()      == DEFAULT_UE_PRI_USER

    # Manual setter pins the value.
    set_priority(77)
    assert get_priority_mode() == "Manual"
    assert get_priority()      == 77

    # The context manager overrides then restores the previous mode/value.
    with priority(123):
        assert get_priority_mode() == "Manual"
        assert get_priority()      == 123
    assert get_priority_mode() == "Manual"     # restored to the manual 77
    assert get_priority()      == 77

    set_priority_auto()                          # leave the global clean for others
    assert get_priority_mode() == "Auto"


def test_par_same_reg_different_priority_orders_writes():
    # Two parallel writes to the SAME register, each at a different priority. The
    # higher-priority write must be emitted LAST inside the reg's always block, so
    # under non-blocking semantics it dominates the lower-priority one.
    reset()

    class worker(Module):
        @flow
        def f(self):
            r       = reg(8)            # REG_reg0  — the shared destination
            hi, lo  = wire(8), wire(8)  # WIRE_wire0 (hi-pri src), WIRE_wire1 (lo-pri src)
            with par():
                with priority(100):
                    r |= hi
                with priority(50):
                    r |= lo

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")       # destructive: moves the arena out
    text = open(os.path.join(out_dir, "top.v")).read()

    # Both assignments target the same reg; the priority-100 source (wire0) must
    # appear after the priority-50 source (wire1) so the higher priority wins.
    reg_writes = [ln for ln in text.splitlines() if "REG_reg0" in ln and "<=" in ln]
    assert len(reg_writes) == 2
    assert "WIRE_wire1" in reg_writes[0]   # priority 50 first
    assert "WIRE_wire0" in reg_writes[1]   # priority 100 last (dominates)

    set_priority_auto()                     # reset global state for later tests


def test_reg_reset_and_wire_default_emit():
    # A reg given reset(v) gets a top-priority clocked write of v; a wire given
    # default(v) gets an internal-low-priority combinational fallback write of v.
    # Raw ints are accepted directly — auto-wrapped into a val sized to the dest.
    reset()

    class worker(Module):
        @init
        def decl(self):
            self.r = reg(8)
            self.w = wire(8)

        @flow
        def f(self):
            self.r.reset(0)         # direct int reset value
            self.w.default(5)       # direct int default value

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text  = open(os.path.join(out_dir, "top.v")).read()
    lines = text.splitlines()

    # Reg reset: exactly one clocked non-blocking write of the literal 0.
    reg_writes = [ln for ln in lines if "REG_reg0" in ln and "<=" in ln]
    assert len(reg_writes) == 1
    assert "8'h0" in text                           # reset literal 0 declared

    # Wire default: exactly one write of the literal 5.
    wire_writes = [ln for ln in lines if "WIRE_wire0" in ln and "<=" in ln]
    assert len(wire_writes) == 1
    assert "8'h5" in text                           # default literal 5 declared


def test_reset_accepts_value_wider_than_64_bits():
    # A >64-bit literal is split into u64 limbs and emitted as a sized hex constant.
    reset()
    big = (1 << 100) | 0xABCDEF        # bit 100 set + low nibble pattern

    class worker(Module):
        @init
        def decl(self):
            self.r = reg(128)

        @flow
        def f(self):
            self.r.reset(big)             # direct big int — no manual val(...) needed

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()

    # Verilog sized hex literal: 128'h<value> with the bit-100 limb pattern intact.
    assert f"128'h{big:x}" in text


def test_reset_dominates_user_write_on_reg():
    # reset(v) is bound at DEFAULT_UE_PRI_RST (max), so it sorts LAST in the reg's
    # always block and dominates a lower-priority user assignment.
    reset()

    class worker(Module):
        @flow
        def f(self):
            r = reg(8)              # REG_reg0 — shared destination
            d = wire(8)             # WIRE_wire0 — user-write source
            r.reset(0)              # reset source (direct int)
            with seq():
                r |= d              # ordinary user write (priority USER)

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    lines = open(os.path.join(out_dir, "top.v")).read().splitlines()

    reg_writes = [ln for ln in lines if "REG_reg0" in ln and "<=" in ln]
    assert len(reg_writes) == 2
    assert "WIRE_wire0" in reg_writes[0]            # user write first (lower priority)
    assert "VAL_"       in reg_writes[1]            # reset last (max priority, dominates)


def test_reset_default_guards():
    # reset(...) only on a reg; default(...) only on a wire. Declare the signals
    # inside a module @init scope (eager), then check the guards on the saved refs.
    reset()
    h = {}

    class worker(Module):
        @init
        def decl(self):
            h["r"], h["w"]   = reg(8), wire(8)
            h["rv"], h["dv"] = val(8, 0), val(8, 1)

    worker()                    # @init runs eagerly, opening/closing its own scope

    with pytest.raises(TypeError):
        h["w"].reset(h["rv"])   # reset on a wire is wrong
    with pytest.raises(TypeError):
        h["r"].default(h["dv"]) # default on a reg is wrong


def test_pick_block_builds_and_gates_branches():
    # `pick` runs whichever pif branch matches; pidef runs when none match. The
    # exit is NOT auto-synchronized (a stderr warning is emitted at build time).
    reset()

    class worker(Module):
        @flow
        def f(self):
            a, b, r = reg(8), reg(8), reg(8)
            with pick():
                with pif(a < b):
                    with seq():        # branch bodies hold sub-blocks, not direct nodes
                        r |= a
                with pif(a > b):
                    with seq():
                        r |= b
                with pidef():
                    with seq():
                        r |= a + b

    set_top(worker())
    gen_flow()
    build_flow()                       # must not panic

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()

    # All three branch destinations write the shared reg; the build completed.
    assert text.count("REG_reg2") >= 3   # r is the third declared reg


def test_pick_rejects_two_defaults():
    # At most one pidef default is allowed; a second one trips the host assert
    # during build_flow.
    reset()

    class worker(Module):
        @flow
        def f(self):
            a, b, r = reg(8), reg(8), reg(8)
            with pick():
                with pif(a < b):
                    with seq():
                        r |= a
                with pidef():
                    with seq():
                        r |= b
                with pidef():
                    with seq():
                        r |= a

    set_top(worker())
    gen_flow()
    with pytest.raises(BaseException):
        build_flow()


def test_build_flow_runs_end_to_end():
    # gen_flow constructs the deferred flow blocks; build_flow then runs the host
    # build pass (schematics, update events, clk / master-reset wiring) over the
    # whole module tree, starting from the explicit top set in setup_function.
    class worker(Module):
        @flow
        def f(self):
            a, b, c = reg(8), wire(8), reg(8)
            with seq():
                c |= a + b

    worker()
    gen_flow()             # construct flow blocks across every module
    build_flow()           # host build pass from the top module


# ---- int-literal operands (auto-wrapped into width-matched val) -------------

def test_int_literal_operands_build_expressions():
    # signal ∘ int and int ∘ signal both auto-wrap the int into a val sized to
    # the signal operand. Reflected ops keep operand order (`5 - a`, not `a - 5`).
    reset()
    seen = {}

    class worker(Module):
        @init
        def decl(self):
            self.a = reg(8)
            self.r = reg(8)

        @flow
        def f(self):
            a = self.a
            seen["add"]  = a + 2          # signal + int
            seen["radd"] = 2 + a          # int + signal  (__radd__)
            seen["rsub"] = 5 - a          # non-commutative reflected (__rsub__)
            seen["band"] = a & 0xF        # bitwise with int
            seen["shl"]  = a << 1         # shift by int
            seen["eq"]   = a == 3         # relational with int
            seen["rlt"]  = 3 < a          # auto-reflected compare -> a.__gt__(3)
            with seq():
                self.r |= a + 1           # int operand inside a clocked assign

    set_top(worker())
    gen_flow()
    assert seen and all(e.hw_type == "EXPR" for e in seen.values())
    build_flow()                          # full host build must not panic


def test_int_literal_assignment_emits_matched_width_const():
    # `r |= 5` wraps the int into a val sized to the destination (12 bits here),
    # so the emitted Verilog constant is a 12-bit sized literal.
    reset()

    class worker(Module):
        @init
        def decl(self):
            self.r = reg(12)

        @flow
        def f(self):
            with seq():
                self.r |= 5               # bare int source -> val(12, 5)

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "12'h5" in text                # constant sized to the destination width


def test_int_literal_wider_than_128_bits():
    # The literal crosses as an arbitrary-precision BigInt, so a >128-bit constant
    # auto-wraps with no manual val() — emitted as a width-sized hex literal.
    reset()
    big = (1 << 200) | 0xABCDEF        # needs > 128 bits to represent

    class worker(Module):
        @init
        def decl(self):
            self.r = reg(256)

        @flow
        def f(self):
            with seq():
                self.r |= big             # bare 201-bit int source -> val(256, big)

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert f"256'h{big:x}" in text        # full bit-200 pattern survives intact


def test_negative_int_literal_wraps_twos_complement():
    # A negative literal masks to its two's-complement value at the dest width.
    reset()

    class worker(Module):
        @init
        def decl(self):
            self.r = reg(8)

        @flow
        def f(self):
            with seq():
                self.r |= -1              # -> val(8, 0xFF)

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "8'hff" in text                # -1 in 8 bits = 0xFF


def test_two_int_operands_rejected():
    # An expression needs at least one signal; two folded ints never reach the
    # connector through normal Python, but a direct mk_expression call is rejected.
    reset()
    ar = k._session.arena()
    with pytest.raises(ValueError):
        ar.mk_expression("x", int(k.LogicOp.ArithPlus), 1, 2)


# ---- Karray (typed multi-dimensional array CCP) -----------------------------

def test_karray_field_is_its_own_hcp():
    # Each field is a distinct HCP sized to the field width (not a bit-slice of a
    # packed element); elem_width is the sum of field widths.
    reset()

    class RobEntry(Karray):
        valid   = kaf(1)
        reg_idx = kaf(5)

    class worker(Module):
        @init
        def decl(self):
            self.rob = RobEntry(HwComponentType.REG, (5, 3), "rob")

    w  = worker()
    ar = k._session.arena()
    valid = w.rob[2][1].valid._to_read_ref()
    ridx  = w.rob[2][1].reg_idx._to_read_ref()
    assert ar.get_hw_bit_sz(valid._ident) == 1       # valid   -> its own 1-bit HCP
    assert ar.get_hw_bit_sz(ridx._ident) == 5        # reg_idx -> its own 5-bit HCP
    assert valid._ident.global_id != ridx._ident.global_id   # distinct hardware
    with pytest.raises(ValueError):
        w.rob[2][1].nope._to_read_ref()              # unknown field rejected at resolve


def test_karray_object_oriented_declaration():
    # Karray subclasses can declare their element record as class fields, keeping
    # the construction call focused on shape/backing/name.
    reset()

    class Src(Karray):
        valid = kaf(1)
        data  = kaf(8)
        note  = kaf(4)

    class worker(Module):
        @init
        def decl(self):
            self.src = Src(HwComponentType.REG, (4,), "src")

    w  = worker()
    ar = k._session.arena()
    valid = w.src[0].valid._to_read_ref()
    data  = w.src[0].data._to_read_ref()
    note  = w.src[0].note._to_read_ref()
    assert ar.get_hw_bit_sz(valid._ident) == 1
    assert ar.get_hw_bit_sz(data._ident)  == 8
    assert ar.get_hw_bit_sz(note._ident)  == 4


def test_karray_reg_backing_emits_per_element_regs():
    # Reg backing materialises one reg per (element, field) — 15x2 for a 5x3 of two
    # fields; a whole element (per-field named sources) and a single field both assign with |=.
    reset()

    class RobEntry(Karray):
        valid   = kaf(1)
        reg_idx = kaf(5)

    class worker(Module):
        @init
        def decl(self):
            self.rob  = RobEntry(HwComponentType.REG, (5, 3), "rob")
            self.vsrc = reg(1)
            self.isrc = reg(5)
            self.vbit = reg(1)

        @flow
        def f(self):
            with seq():
                self.rob[2][1] |= {"valid": self.vsrc, "reg_idx": self.isrc}   # whole element (flat idx 7), per field
                self.rob[0][0].valid |= self.vbit                              # the valid field's own 1-bit HCP

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    for flat in range(15):
        assert f"rob_E{flat}_" in text               # every per-element reg present


def test_karray_wire_backing_uses_imul():
    # Wire backing is combinational — assign with *=; |= must be rejected.
    reset()

    class BusEntry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.bus = BusEntry(HwComponentType.WIRE, (2, 2), "bus")
            self.s   = reg(8)

        @flow
        def f(self):
            with seq():
                self.bus[1][0] *= {"data": self.s}   # element (1,0) -> flat 2

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "bus_E2_" in text


def test_karray_memblock_backing_declares_block():
    # MemBlock backing folds the array onto one addressable block; a write builds
    # a write MemEle at the constant flattened address.
    reset()

    class RobEntry(Karray):
        valid   = kaf(1)
        reg_idx = kaf(5)

    class worker(Module):
        @init
        def decl(self):
            self.kmem = RobEntry(HwComponentType.MEM_BLOCK, (5, 3), "kmem")
            self.vsrc = reg(1)
            self.isrc = reg(5)

        @flow
        def f(self):
            with seq():
                self.kmem[2][1] |= {"valid": self.vsrc, "reg_idx": self.isrc}   # write element at flat addr 7

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "kmem_valid_MEM"   in text                # one MemBlk per field
    assert "kmem_reg_idx_MEM" in text


def test_karray_backing_enforces_assignment_operator():
    # |= requires reg/mem backing; *= requires wire backing. The guard raises in
    # Python (via the resolved element's clocked-ness) before mutating the model.
    reset()

    class VEntry(Karray):
        v = kaf(4)

    class worker(Module):
        @init
        def decl(self):
            self.rk = VEntry(HwComponentType.REG,  (2,), "rk")
            self.wk = VEntry(HwComponentType.WIRE, (2,), "wk")
            self.s  = reg(4)

        @flow
        def f(self):
            with pytest.raises(TypeError):
                self.wk[0] |= {"v": self.s}          # |= on wire-backed element
            with pytest.raises(TypeError):
                self.rk[0] *= {"v": self.s}          # *= on reg-backed element

    set_top(worker())
    gen_flow()


def test_karray_1d_element_assignment():
    # A 1-D Karray subscript lands directly on the Karray (not a KarrayRef), so the
    # `d[i] |= x` augmented-assign tail goes through Karray.__setitem__. Exercise both
    # a whole-element and a field assign on a 1-D array end to end.
    reset()

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(7)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.hi  = val(1, 1)
            self.dat = reg(7)

        @flow
        def f(self):
            with seq():
                self.rf[2] |= {"valid": self.hi, "data": self.dat}   # whole element (per-field named sources)
                self.rf[0].valid |= self.hi          # 1-D + field
                self.rf[0].data  |= self.dat

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "rf_E2_valid" in text and "rf_E2_data" in text   # element 2 split across fields
    assert "rf_E0_valid" in text and "rf_E0_data" in text   # element 0 field-wise


# ---- Karray dynamic (runtime-signal) indexing -------------------------------

def test_karray_dynamic_binary_read():
    # A bare signal index = binary-encoded address. Reading a field of the
    # dynamically-selected element yields a fresh mux-output wire of the field width.
    # The read MATERIALISES mux hardware, so it runs inside the module scope (@init).
    reset()
    res = {}

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.sel = reg(2)                       # 2-bit binary address for 4 elements
            res["got"] = self.rf[self.sel].data._to_read_ref()   # dynamic read of .data

    worker()
    ar  = k._session.arena()
    got = res["got"]
    assert ar.get_hw_bit_sz(got._ident) == 8       # mux output = field width
    assert got._ident.hw_type == "WIRE"            # combinational mux result


def test_karray_dynamic_onehot_read():
    # oh(sig) marks the index as one-hot: one select bit per element.
    reset()
    res = {}

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.sel = reg(4)                       # 4-bit one-hot for 4 elements
            res["got"] = self.rf[oh(self.sel)].data._to_read_ref()

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["got"]._ident) == 8


def test_karray_dynamic_mixed_static_dynamic_read():
    # 2-D array: one static dim + one dynamic dim resolves to a single element.
    reset()
    res = {}

    class Cell(Karray):
        v = kaf(1)
        d = kaf(6)

    class worker(Module):
        @init
        def decl(self):
            self.grid = Cell(HwComponentType.REG, (3, 4), "grid")
            self.sel  = reg(2)
            res["got"] = self.grid[2][self.sel].d._to_read_ref()   # row 2 static, col dynamic

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["got"]._ident) == 6


def test_karray_dynamic_read_emits_verilog():
    # End-to-end: a dynamic read feeds a reg; the whole build/emit must not panic.
    reset()

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.sel = reg(2)
            self.out = reg(8)

        @flow
        def f(self):
            with seq():
                self.out |= self.rf[self.sel].data

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "rf_E0_data" in text and "rf_E3_data" in text   # all elements wired into the mux


def test_karray_dynamic_write_rejected():
    # Dynamic indexing is read-only; assigning through a dynamic index is rejected.
    reset()

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.sel = reg(2)
            self.src = reg(8)
            # the guard raises in pure Python before any arena work, so it needs no
            # seq scope (and would leave one empty if placed inside).
            with pytest.raises(NotImplementedError):
                self.rf[self.sel].data |= self.src

    worker()


def test_karray_dynamic_narrow_selector_rejected():
    # A binary selector too narrow to address the dimension is rejected.
    reset()

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")   # needs >= 2 bits
            self.sel = reg(1)                                     # only 1 bit

    w = worker()
    with pytest.raises(BaseException):                            # host assert -> panic/exception
        w.rf[w.sel].data._to_read_ref()


# ---- Karray callback-driven reduce ------------------------------------------

def test_karray_reduce_max_by_field():
    # reduce a 1-D regfile to the element with the largest `data` (a >= b picks left).
    reset()
    res = {}

    class RegFile(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = RegFile(HwComponentType.REG, (4,), "rf")
            res["w"] = self.rf.reduce([Reduce], lambda a, b, lvl: a.fields["data"] >= b.fields["data"])

    worker()
    ar  = k._session.arena()
    got = res["w"].data._to_read_ref()
    assert ar.get_hw_bit_sz(got._ident) == 8       # winner field = field width
    assert got._ident.hw_type == "WIRE"            # mux-tree output


def test_karray_reduce_odd_length_carry():
    # odd element count exercises the "carry-up" of the unpaired node.
    reset()
    res = {}

    class RegFile(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = RegFile(HwComponentType.REG, (3,), "rf")
            res["w"] = self.rf.reduce([Reduce], lambda a, b, lvl: a.fields["data"] >= b.fields["data"])

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["w"].data._to_read_ref()._ident) == 8


def test_karray_reduce_2d_mixed_pin_and_fold():
    # 2-D array: pin one dim (row 1), fold the other — reduce that row only.
    reset()
    res = {}

    class Cell(Karray):
        data = kaf(6)

    class worker(Module):
        @init
        def decl(self):
            self.grid = Cell(HwComponentType.REG, (2, 3), "grid")
            res["w"] = self.grid.reduce([1, Reduce], lambda a, b, lvl: a.fields["data"] >= b.fields["data"])

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["w"].data._to_read_ref()._ident) == 6


def test_karray_reduce_emits_verilog():
    # End-to-end: reduce feeds a reg; the whole build/emit must not panic.
    reset()

    class RegFile(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RegFile(HwComponentType.REG, (4,), "rf")
            self.out = reg(8)

        @flow
        def f(self):
            with seq():
                w = self.rf.reduce([Reduce], lambda a, b, lvl: a.fields["data"] >= b.fields["data"])
                self.out |= w.data

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "rf_E0_data" in text and "rf_E3_data" in text   # all elements fed into the reduce


# ---- reduce: per-dim functions / extras / request_index ----------------------

def test_karray_reduce_per_dim_nested():
    # 2-D: each folded dim reduces with its own fn (Reduce(fn)). Nested.
    reset()
    res = {}

    class Cell(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.grid = Cell(HwComponentType.REG, (2, 3), "grid")
            res["w"] = self.grid.reduce([
                Reduce(lambda a, b, l: a.fields["data"] <= b.fields["data"]),   # rows: min
                Reduce(lambda a, b, l: a.fields["data"] >= b.fields["data"]),   # cols: max
            ])

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["w"].data._to_read_ref()._ident) == 8


def test_karray_reduce_request_index_coords():
    # request_index returns (winner, coords): one index signal per folded dim.
    reset()
    res = {}

    class Cell(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.grid = Cell(HwComponentType.REG, (2, 3), "grid")
            w, coords = self.grid.reduce([Reduce, Reduce],
                                         lambda a, b, l: a.fields["data"] >= b.fields["data"],
                                         request_index=True)
            res["w"], res["coords"] = w, coords

    worker()
    ar = k._session.arena()
    coords = res["coords"]
    assert len(coords) == 2                            # one per folded dim
    assert ar.get_hw_bit_sz(coords[0]._ident) == 1     # dim0 extent 2 -> 1 bit
    assert ar.get_hw_bit_sz(coords[1]._ident) == 2     # dim1 extent 3 -> 2 bits


def test_karray_reduce_extras_carry():
    # select_fn may return (select, {name: signal}); extras are visible next layer.
    reset()
    res = {}

    class Rf(Karray):
        data = kaf(8)

    def pick_sum(a, b, level):
        asum = a.fields.get("runsum", a.fields["data"])   # seed from data at leaves
        bsum = b.fields.get("runsum", b.fields["data"])
        return (asum >= bsum), {"runsum": asum + bsum}

    class worker(Module):
        @init
        def decl(self):
            self.rf = Rf(HwComponentType.REG, (4,), "rf")
            res["w"] = self.rf.reduce([Reduce], pick_sum)

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["w"].data._to_read_ref()._ident) == 8


def test_karray_reduce_fold_needs_fn():
    # a bare Reduce with no select_fn is an error.
    reset()

    class Rf(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = Rf(HwComponentType.REG, (4,), "rf")
            with pytest.raises(TypeError):
                self.rf.reduce([Reduce])              # no per-dim fn, no select_fn

    worker()

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
    Karray, KBundle, kaf, HwComponentType,
    any_of, mux, rotate_left, sum_cnt,
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
# Every access selects each dimension with ONE of the four unified index kinds:
#   d[3]      static int          d[sig]   dynamic binary address
#   d[2, 5]   inclusive range     d[fn]    custom fn(i) -> 1-bit enable

def test_karray_field_is_its_own_hcp():
    # Each field is a distinct HCP sized to the field width (not a bit-slice of a
    # packed element).
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
    # Karray subclasses declare their element record as class fields, keeping the
    # construction call focused on shape/backing/name.
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
    assert ar.get_hw_bit_sz(w.src[0].valid._to_read_ref()._ident) == 1
    assert ar.get_hw_bit_sz(w.src[0].data._to_read_ref()._ident)  == 8
    assert ar.get_hw_bit_sz(w.src[0].note._to_read_ref()._ident)  == 4


def test_karray_widths_may_be_set_at_instantiation():
    # A kaf() width in a class body is a DEFAULT: the same record class serves
    # any width, which is what a generator sizing arrays from a description
    # needs. Without this, one shape at two widths meant two classes.
    reset()

    class Entry(Karray):
        pc    = kaf(32)                 # default
        instr = kaf(32)

    class worker(Module):
        @init
        def decl(self):
            self.wide = Entry(HwComponentType.REG, (2,), "wide", pc=64, instr=16)
            self.dflt = Entry(HwComponentType.REG, (1,), "dflt")

    w  = worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(w.wide[0].pc._to_read_ref()._ident)    == 64
    assert ar.get_hw_bit_sz(w.wide[1].instr._to_read_ref()._ident) == 16
    # One class, two live arrays, unrelated widths — and the class is untouched.
    assert ar.get_hw_bit_sz(w.dflt[0].pc._to_read_ref()._ident)    == 32
    assert Entry.__karray_fields__ == (("pc", 32), ("instr", 32))


def test_karray_field_declared_without_a_width_must_be_given_one():
    # kaf() with no width is a record saying "this number is the caller's".
    reset()

    class Entry(Karray):
        tag  = kaf(4)
        data = kaf()                    # no default: every instantiation says

    assert Entry.__karray_fields__ == (("tag", 4), ("data", None))

    class worker(Module):
        @init
        def decl(self):
            self.ok = Entry(HwComponentType.REG, (2,), "ok", data=12)

    w = worker()
    assert k._session.arena().get_hw_bit_sz(w.ok[0].data._to_read_ref()._ident) == 12

    class lazy(Module):
        @init
        def decl(self):
            self.bad = Entry(HwComponentType.REG, (2,), "bad")

    with pytest.raises(TypeError, match="must give it one"):
        lazy()


def test_karray_width_override_is_checked():
    reset()

    class Entry(Karray):
        data = kaf(8)

    def build(**widths):
        class worker(Module):
            @init
            def decl(self):
                self.e = Entry(HwComponentType.REG, (2,), "e", **widths)
        return worker()

    with pytest.raises(TypeError, match="no field named 'dat'"):
        build(dat=16)                   # a typo cannot silently do nothing
    with pytest.raises(TypeError, match="must be an int"):
        build(data="16")
    with pytest.raises(ValueError, match="must be >= 1"):
        build(data=0)


def test_karray_field_may_be_added_at_instantiation():
    # The keyword's VALUE picks: an int sets a declared field's width, a kaf()
    # adds a field only this array has. One record class, two shapes.
    reset()

    class Entry(Karray):
        pc    = kaf(32)
        instr = kaf(32)

    class worker(Module):
        @init
        def decl(self):
            self.spec  = Entry(HwComponentType.REG, (2,), "spec",
                               pc=64, spectag=kaf(8))
            self.plain = Entry(HwComponentType.REG, (1,), "plain")

    w  = worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(w.spec[0].pc._to_read_ref()._ident)      == 64
    assert ar.get_hw_bit_sz(w.spec[1].spectag._to_read_ref()._ident) == 8
    # The class is never mutated: a sibling array has neither the width nor the
    # added field.
    assert Entry.__karray_fields__ == (("pc", 32), ("instr", 32))
    assert ar.get_hw_bit_sz(w.plain[0].pc._to_read_ref()._ident) == 32
    with pytest.raises(ValueError):
        w.plain[0].spectag._to_read_ref()


def test_karray_added_field_may_be_a_bundle():
    # An added field flattens through the same walk a declared one takes, so the
    # attribute chain reads it back identically.
    reset()

    class Vec2(KBundle):
        x = kaf(4)
        y = kaf(6)

    class Entry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.e = Entry(HwComponentType.REG, (2,), "e", pos=kaf(Vec2))

    w  = worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(w.e[0].pos.x._to_read_ref()._ident) == 4
    assert ar.get_hw_bit_sz(w.e[0].pos.y._to_read_ref()._ident) == 6


def test_karray_added_field_is_checked():
    reset()

    class Vec2(KBundle):
        x = kaf(4)

    class Entry(Karray):
        data  = kaf(8)
        pos_x = kaf(2)          # a literal leaf an added bundle would collide with

    def build(**fields):
        class worker(Module):
            @init
            def decl(self):
                self.e = Entry(HwComponentType.REG, (2,), "e", **fields)
        return worker()

    with pytest.raises(TypeError, match="already declared on the class"):
        build(data=kaf(16))     # use data=16 to change a declared field
    with pytest.raises(TypeError, match="an added field must state one"):
        build(spectag=kaf())    # nothing left to size it later
    with pytest.raises(TypeError, match="duplicate field name 'pos_x'"):
        build(pos=kaf(Vec2))    # flattens onto the declared leaf


def test_karray_field_may_not_shadow_a_constructor_parameter():
    # Overrides ride in as kwargs, so `Entry(REG, (2,), name=8)` must not be
    # able to mean two things. Caught when the class is written.
    reset()
    with pytest.raises(TypeError, match="collide with Karray.__init__"):
        class Bad(Karray):
            name = kaf(8)


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


def test_karray_memblock_backing_rejected():
    # MemBlock backing was removed — only Reg and Wire may back a Karray.
    reset()

    class RobEntry(Karray):
        valid = kaf(1)

    class worker(Module):
        @init
        def decl(self):
            with pytest.raises(ValueError):
                RobEntry(HwComponentType.MEM_BLOCK, (4,), "kmem")

    worker()


def test_karray_backing_enforces_assignment_operator():
    # |= requires the reg backing; *= requires the wire backing. The guard raises
    # before mutating the model.
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


def test_karray_bare_assign_rejected():
    # A bare `=` carries no clocked/comb intent — every Karray assign must pick
    # `|=` or `*=` explicitly.
    reset()

    class RfEntry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = RfEntry(HwComponentType.REG, (4,), "rf")
            self.s  = reg(8)
            with pytest.raises(TypeError):
                self.rf[0] = {"data": self.s}
            with pytest.raises(TypeError):
                self.rf[0].data = self.s

    worker()


def test_karray_colon_slice_rejected():
    # Ranges are not supported at all — a colon slice raises at the subscript.
    reset()

    class RfEntry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = RfEntry(HwComponentType.REG, (4,), "rf")
            with pytest.raises(TypeError):
                self.rf[0:2]

    worker()


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


def test_karray_scalar_and_int_sources():
    # A single-field Karray takes a bare scalar source (the sole field is implied),
    # and int literals wrap into field-width vals — both alone and inside a map.
    reset()

    class RfEntry(Karray):
        data = kaf(8)

    class TwoField(Karray):
        a = kaf(4)
        b = kaf(4)

    class worker(Module):
        @init
        def decl(self):
            self.rf = RfEntry(HwComponentType.REG, (4,), "rf")
            self.tf = TwoField(HwComponentType.REG, (2,), "tf")
            self.s  = reg(8)

        @flow
        def f(self):
            with seq():
                self.rf[0] |= self.s                 # bare signal -> sole field
                self.rf[1] |= 5                      # bare int    -> sole field, width-matched val
                self.tf[0] |= {"a": 3, "b": self.s}  # int inside a map (b auto-resizes)
                with pytest.raises(TypeError):
                    self.tf[1] |= self.s             # bare scalar on a multi-field Karray

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "rf_E0_data" in text and "rf_E1_data" in text


# ---- Karray static ranges (inclusive tuples) --------------------------------

def test_karray_range_index_rejected():
    # Ranges are not supported — every subscript names exactly one element, so a
    # (start, stop) tuple raises at the subscript itself.
    reset()

    class Entry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = Entry(HwComponentType.REG, (4,), "rf")
            with pytest.raises(TypeError, match="ranges are not supported"):
                self.rf[0, 1]

    worker()


def test_karray_under_indexed_rejected():
    # Every dimension must be indexed: a 2-D Karray accessed with one index is a
    # rank error at statement time.
    reset()

    class Entry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = Entry(HwComponentType.REG, (2, 3), "rf")
            self.s  = reg(8)

        @flow
        def f(self):
            with seq():
                self.rf[0].data |= self.s            # dim 1 never indexed

    set_top(worker())
    with pytest.raises(ValueError, match="index every dimension"):
        gen_flow()


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


def test_karray_reduce_read_max():
    # A callable index on the READ side folds the dim through a REDUCE tree: the
    # fn is a pair-select `fn(a, b, level) -> pick-a` over ReduceViews. Reading a
    # field of the winner yields a fresh mux-output wire of the field width.
    reset()
    res = {}

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = RfEntry(HwComponentType.REG, (4,), "rf")
            res["got"] = self.rf[lambda a, b, l: a.fields["data"] >= b.fields["data"]].data._to_read_ref()

    worker()
    ar  = k._session.arena()
    got = res["got"]
    assert ar.get_hw_bit_sz(got._ident) == 8
    assert got._ident.hw_type == "WIRE"            # reduce-tree mux output


def test_karray_reduce_read_odd_length_carry():
    # An odd element count exercises the carry-up of the unpaired node.
    reset()
    res = {}

    class RfEntry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf = RfEntry(HwComponentType.REG, (3,), "rf")
            res["got"] = self.rf[lambda a, b, l: a.fields["data"] >= b.fields["data"]].data._to_read_ref()

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["got"]._ident) == 8


def test_karray_reduce_2d_pin_and_fold():
    # 2-D: pin one dim (row 1), reduce the other — only that row folds. The
    # select fn sees the covered indices of the folding dim on each side.
    reset()
    res = {}
    covered = []

    class Cell(Karray):
        d = kaf(6)

    def pick(a, b, level):
        covered.append((list(a.indices), list(b.indices), level))
        return a.fields["d"] >= b.fields["d"]

    class worker(Module):
        @init
        def decl(self):
            self.grid = Cell(HwComponentType.REG, (2, 3), "grid")
            res["got"] = self.grid[1][pick].d._to_read_ref()

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["got"]._ident) == 6
    # 3 children -> pairs (0,1) at level 0, then (01, 2) at level 1
    assert covered == [([0], [1], 0), ([0, 1], [2], 1)]


def test_karray_reduce_extras_carry():
    # A select fn may return (select, {name: signal}); an extra replaces a
    # same-named carried field, so the next level (and the result) see it.
    reset()
    res = {}

    class Rf(Karray):
        data = kaf(8)

    def pick_sum(a, b, level):
        asum = a.fields["data"]
        bsum = b.fields["data"]
        return (asum >= bsum), {"data": asum + bsum}   # winner's data = running sum

    class worker(Module):
        @init
        def decl(self):
            self.rf = Rf(HwComponentType.REG, (4,), "rf")
            res["got"] = self.rf[pick_sum].data._to_read_ref()

    worker()
    ar = k._session.arena()
    assert ar.get_hw_bit_sz(res["got"]._ident) == 8
    assert res["got"]._ident.hw_type == "EXPR"     # top extra (a+b) replaced the muxed wire


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


def test_karray_dynamic_binary_write():
    # Binary-address dynamic write: `rf[sel].data |= src` writes only the selected
    # element; others hold. Each element's data reg is emitted (guarded by sel==k).
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

        @flow
        def f(self):
            with seq():
                self.rf[self.sel].data |= self.src

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "rf_E0_data" in text and "rf_E3_data" in text   # every element guarded-driven


def test_karray_cus_fn_write():
    # A callable index on a write gates each element with its fn(i) enable — the
    # custom kind subsumes both one-hot writes (fn = lambda i: sel[i]) and the old
    # coordinate-callback assign (fn = lambda i: sel == i).
    reset()

    class RfEntry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.oh  = reg(4)                       # one-hot select line
            self.sel = reg(2)                       # binary index for the compare form
            self.src = reg(8)

        @flow
        def f(self):
            with seq():
                self.rf[lambda i: self.oh[i]].data  |= self.src     # one-hot style
                self.rf[lambda i: self.sel == i].data |= self.src   # compare style

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "rf_E0_data" in text and "rf_E3_data" in text


def test_karray_dynamic_write_map():
    # Whole-element map write into the runtime-selected element (per-field by name).
    reset()

    class RfEntry(Karray):
        valid = kaf(1)
        data  = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.sel = reg(2)
            self.v   = reg(1)
            self.d   = reg(8)

        @flow
        def f(self):
            with seq():
                self.rf[self.sel] |= {"valid": self.v, "data": self.d}

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "rf_E0_valid" in text and "rf_E0_data" in text


def test_karray_dynamic_write_on_wire_rejected():
    # A runtime-collapsed (dynamic/custom) write needs a reg backing: non-selected
    # elements hold, and a wire cannot hold.
    reset()

    class RfEntry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.WIRE, (4,), "rf")
            self.sel = reg(2)
            self.src = reg(8)
            with pytest.raises(TypeError):
                self.rf[self.sel].data *= self.src

    worker()


def test_karray_dynamic_narrow_selector_rejected():
    # A binary selector too narrow to address the dimension is a clean ValueError.
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
    with pytest.raises(ValueError):
        w.rf[w.sel].data._to_read_ref()


def test_karray_cus_fn_wide_enable_rejected():
    # A custom WRITE index fn must return a 1-bit enable per index.
    reset()

    class RfEntry(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.rf  = RfEntry(HwComponentType.REG, (4,), "rf")
            self.src = reg(8)
            with pytest.raises(TypeError):
                self.rf[lambda i: self.src].data |= self.src      # 8-bit enable

    worker()


# ---- Karray mixed-kind k2k (the full unified form) ---------------------------

def test_karray_mixed_kinds_k2k():
    # All three kinds in ONE statement, on both sides:
    #   a[1][2][1][dyn_w]  |=  b[reduce_fn][dyn_r][1]
    # the source's reduce dim folds at runtime (select fn per pair) and its dyn
    # dim muxes; the destination's dynamic dim guards with write enables.
    reset()

    class Cell(Karray):
        data = kaf(8)

    def pick_max(av, bv, level):
        return av.fields["data"] >= bv.fields["data"]

    class worker(Module):
        @init
        def decl(self):
            self.a  = Cell(HwComponentType.REG, (2, 3, 2, 3), "a")
            self.b  = Cell(HwComponentType.REG, (2, 3, 2), "b")
            self.aw = reg(2)                         # binary address into a's dim 3 (extent 3)
            self.br = reg(2)                         # binary address into b's dim 1 (extent 3)

        @flow
        def f(self):
            with seq():
                self.a[1][2][1][self.aw] |= self.b[pick_max][self.br][1]

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "a_E" in text and "b_E" in text


def test_karray_k2k_dynamic_element_source():
    # A dynamically-selected source ELEMENT feeds a static destination element —
    # k2k with a runtime source side (rejected by the old design, native now).
    reset()

    class Cell(Karray):
        data = kaf(8)

    class worker(Module):
        @init
        def decl(self):
            self.dst = Cell(HwComponentType.REG, (4,), "dst")
            self.src = Cell(HwComponentType.REG, (4,), "src")
            self.sel = reg(2)

        @flow
        def f(self):
            with seq():
                self.dst[0] |= self.src[self.sel]

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")


# ---- Karray bundles (nested kaf, Chisel-Bundle style) ------------------------

def test_karray_bundle_nested_fields():
    # kaf(Bundle) flattens the bundle's fields under the spec name; bundles nest
    # inside bundles; attribute chains walk into the flat leaf names.
    reset()
    from kathryn import KBundle

    class Vec2(KBundle):
        x = kaf(8)
        y = kaf(8)

    class Payload(KBundle):
        pos = kaf(Vec2)              # bundle inside bundle
        tag = kaf(4)

    class Entry(Karray):
        valid = kaf(1)
        pay   = kaf(Payload)

    assert Entry.__karray_fields__ == (
        ("valid", 1), ("pay_pos_x", 8), ("pay_pos_y", 8), ("pay_tag", 4))

    class worker(Module):
        @init
        def decl(self):
            self.e = Entry(HwComponentType.REG, (2,), "e")

    w  = worker()
    ar = k._session.arena()
    x = w.e[0].pay.pos.x._to_read_ref()          # chained attrs -> flat leaf "pay_pos_x"
    assert ar.get_hw_bit_sz(x._ident) == 8
    t = w.e[1].pay.tag._to_read_ref()
    assert ar.get_hw_bit_sz(t._ident) == 4
    with pytest.raises(ValueError):
        w.e[0].pay.pos.z._to_read_ref()          # no such leaf


def test_karray_bundle_duplicate_flat_name_rejected():
    # A literal leaf colliding with a bundle's flattened name is caught at class
    # declaration time.
    reset()
    from kathryn import KBundle

    class Vec(KBundle):
        x = kaf(8)

    with pytest.raises(TypeError):
        class Bad(Karray):
            pos   = kaf(Vec)         # -> pos_x
            pos_x = kaf(8)           # collides


def test_karray_bundle_map_assign_and_k2k():
    # Nested dict sources flatten to the leaf names; a bundle FIELD target takes
    # a sub-field map; k2k pairs bundles structurally (same flat names+widths).
    reset()
    from kathryn import KBundle

    class Vec2(KBundle):
        x = kaf(8)
        y = kaf(8)

    class Entry(Karray):
        valid = kaf(1)
        pos   = kaf(Vec2)

    class worker(Module):
        @init
        def decl(self):
            self.a  = Entry(HwComponentType.REG, (2,), "a")
            self.b  = Entry(HwComponentType.REG, (2,), "b")
            self.v  = reg(1)
            self.sx = reg(8)
            self.sy = reg(8)

        @flow
        def f(self):
            with seq():
                # whole element via nested dicts
                self.a[0] |= {"valid": self.v, "pos": {"x": self.sx, "y": self.sy}}
                # bundle-field target via a sub-field map (+ int literal)
                self.a[1].pos |= {"x": self.sx, "y": 7}
                # leaf write through the chain
                self.a[1].pos.y |= self.sy
                # k2k: bundles pair by flat name+width
                self.b[0] |= self.a[0]

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    assert "a_E0_pos_x" in text and "a_E0_pos_y" in text
    assert "b_E0_pos_x" in text




# ---- combinational combinators (built in the Rust core, arena_impl_comb.rs) --


def test_comb_pure_expression_widths_and_identities():
    # rotate_left/any_of/sum_cnt are pure expressions but still declare
    # expression HCPs, so they run inside a module scope (@init is enough).
    h = {}

    class worker(Module):
        @init
        def decl(self):
            a    = reg(8, "a")
            sel  = reg(1, "sel")
            bits = [reg(1) for _ in range(5)]
            ar   = k._session.arena()

            # a full (or zero) turn is the identity: the SAME signal comes back
            assert rotate_left(a, 8)._ident.global_id == a._ident.global_id
            assert rotate_left(a, 0)._ident.global_id == a._ident.global_id
            h["rot"] = rotate_left(a, 3)

            # single-term any_of is the identity; empty is a 1-bit constant 0
            assert any_of([sel])._ident.global_id == sel._ident.global_id
            empty = any_of([])
            assert empty.hw_type == "VAL" and ar.get_hw_bit_sz(empty._ident) == 1

            # default sum width is exactly big enough for the largest sum:
            # 5x 1-bit count 0..5 in 3 bits; 4x 8-bit need (4*255).bit_length()=10
            h["w5x1"] = ar.get_hw_bit_sz(sum_cnt(bits)._ident)
            h["w4x8"] = ar.get_hw_bit_sz(sum_cnt([a, a, a, a])._ident)

            # validation now raised from the Rust core
            with pytest.raises(ValueError, match="sum_cnt of no signals"):
                sum_cnt([])
            with pytest.raises(ValueError, match="exceeds the signal's"):
                rotate_left(a, 1, width=9)
            with pytest.raises(ValueError, match="needs a width >= 1"):
                rotate_left(a, 1, width=0)
            with pytest.raises(TypeError, match="amount must be an int"):
                rotate_left(a, 1.5)

    worker()
    assert h["rot"].hw_type == "EXPR"
    assert h["w5x1"] == 3
    assert h["w4x8"] == 10


def test_comb_mux_emits_priority_if_else():
    # mux declares hardware (wire + zif/zelse), so it lives in a flow scope.
    # An int arm is wrapped to the mux width by the connector; two int arms
    # cannot infer a width and must say one.
    reset()

    class worker(Module):
        @init
        def decl(self):
            self.sel  = reg(1, "sel")
            self.a    = reg(8, "a")
            self.b    = reg(8, "b")
            self.o    = wire(8, "o")
            self.o2   = wire(4, "o2")
            self.rot  = wire(8, "rot")
            self.cnt  = wire(3, "cnt")
            self.anyb = wire(1, "anyb")
            self.bits = [reg(1) for _ in range(5)]

        @flow
        def f(self):
            with seq():
                self.o  *= mux(self.sel, self.a, self.b, name="pickab")
                self.o2 *= mux(self.sel, 5, 9, width=4)
                with pytest.raises(TypeError, match="cannot infer a width"):
                    mux(self.sel, 3, 4)              # two ints, no width
                self.rot  *= rotate_left(self.a, 3)
                self.cnt  *= sum_cnt(self.bits)
                self.anyb *= any_of(self.bits)

    set_top(worker())
    gen_flow()
    build_flow()

    out_dir = tempfile.mkdtemp()
    emit_verilog(out_dir, "top")
    text = open(os.path.join(out_dir, "top.v")).read()
    # the named mux wire exists and both arms assign it (zif arm + zelse arm)
    assert "WIRE_pickab" in text
    assert len([ln for ln in text.splitlines()
                if "WIRE_pickab" in ln and "=" in ln and "always" not in ln]) >= 2
    # rotate = shl | shr of the same source
    assert "<<" in text and ">>" in text

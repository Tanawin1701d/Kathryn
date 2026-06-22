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
)


def setup_function(_):
    reset()
    set_top(Module("top"))   # top is explicit — user provides the Module to set as top


def test_singleton_survives_double_import():
    import kathryn as a
    import kathryn as b
    assert a._session.arena() is b._session.arena()


def test_hw_constructors_and_types():
    assert reg(8).hw_type   == "REG"
    assert wire(8).hw_type  == "WIRE"
    assert val(8, 3).hw_type == "VAL"
    blk = mem_blk(8, 4)
    assert blk.hw_type == "MEM_BLOCK"
    idx = val(4, 0)
    ele = mem_ele(blk, idx, 8, True)          # read element
    assert ele.global_id > 0
    iow = wire(1)
    iow.mark_input("test_in")
    assert iow.is_io


def test_optional_names_autogenerate():
    a = reg(8)              # no name -> auto
    b = reg(8, "explicit")
    assert a.global_id != b.global_id


def test_binary_operator_builds_expression():
    a, b = reg(8), wire(8)
    e = a + b
    assert isinstance(e, expr)
    assert e.hw_type == "EXPR"
    # chaining + bitwise + shift + relational all produce expressions
    sh = val(8, 2)                          # operands must be signals, not int literals
    assert ((a & b) >> sh).hw_type == "EXPR"
    assert (a == b).hw_type == "EXPR"


def test_unary_invert_builds_expression():
    a = reg(8)
    assert (~a).hw_type == "EXPR"
    assert a.lnot().hw_type == "EXPR"


def test_inclusive_slicing():
    a = reg(16)
    s = a[8, 0]._slice          # bits 8..0 inclusive
    assert (s.start, s.stop) == (0, 9)
    one = a[3]._slice           # single bit
    assert (one.start, one.stop) == (3, 4)
    with pytest.raises(TypeError):
        a[8:0]                  # python-slice form is rejected


def test_clocked_assignment_with_ior():
    a, b, c = reg(8), wire(8), reg(8)
    with seq():
        c |= a + b              # reg <- expr (clocked)


def test_comb_assignment_with_imul():
    a, b = reg(8), wire(8)
    with seq():
        b *= a                  # wire <- reg (combinational)


def test_assignment_operator_guards():
    # The guard raises in Python before any arena call, so no scope is needed.
    r, w = reg(8), wire(8)
    with pytest.raises(TypeError):
        w |= r                  # |= on a wire is wrong
    with pytest.raises(TypeError):
        r *= w                  # *= on a reg is wrong


def test_sliced_assignment():
    a, b = reg(16), wire(16)
    with seq():
        a[7, 0] |= b[7, 0]      # sliced clocked assign
        b[15, 8] *= a[15, 8]    # sliced comb assign


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
                pass

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
            with seq():
                pass

    class mod_b(Module):
        @flow
        def f(self):
            order.append("b")
            with seq():
                pass

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

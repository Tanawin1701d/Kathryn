# Smoke tests for the Kathryn Python DSL. Each test resets the singleton arena
# first for isolation. Run with: pytest py/tests  (after `maturin develop`).

import pytest
import kathryn as k
from kathryn import (
    reset, reg, wire, val, io_wire, mem_blk, mem_ele,
    seq, sif, expr,
    Module, init, flow, gen_flow,
)


def setup_function(_):
    reset()


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
    src = reg(8)
    iow = io_wire(True, src, src)
    assert iow.hw_type == "IO_WIRE"


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

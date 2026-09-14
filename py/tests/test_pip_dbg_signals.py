# A @dbg body reads a pip's status signals back AFTER build_flow — the arb's
# bound gates (master-ack / hold / reset) and the pip's wait4syn register — as
# SignalRefs with NO new hardware, so a Module attribute holding one reaches
# sim_manifest.json.
#   src  pip(auto_req) > zync(con)             con leaf 0, added BEFORE con's pip
#   con  pip           > zync(sink, auto_ack)  con leaf 1; sink is no_pip_master
# The emit must be byte-identical with and without the @dbg method, compared
# across TWO subprocesses (in-process ids shift between builds).

import filecmp
import json
import os
import subprocess
import sys
import textwrap

import pytest

from kathryn import (Module, PipCon, SignalRef, arena, build_flow, build_model, dbg, emit_verilog,
                     flow, gen_dbg, gen_flow, init, pip, reg, reset, seq, set_top, wire, zync)

ran: list = []      # one entry per @dbg run


class two_pips(Module):
    @init
    def com_declare(self):
        self.src, self.con, self.sink = PipCon(), PipCon(), PipCon()
        self.sink.no_pip_master()
        self.hold_w  = wire(1, "hold_w")
        self.flush_w = wire(1, "flush_w")
        self.count_a = reg(8, "count_a")
        self.count_b = reg(8, "count_b")
        self.tick    = reg(8, "tick")

    @flow
    def my_flow(self):
        self.con.set_hold (self.hold_w)
        self.con.set_reset(self.flush_w)
        with pip(self.src, auto_req=True):
            with zync(self.con):                             # con leaf 0
                self.count_a |= self.count_a + 1
        with pip(self.con):                                  # con leaf 1
            with zync(self.sink, auto_ack=True):
                self.count_b |= self.count_b + 1
        with seq() as plain:                                 # a NON-pip block, for the refusal
            self.tick |= self.tick + 1
        self.seq_i = plain.ident


class two_pips_dbg(two_pips):
    @dbg
    def probe(self):
        ran.append(1)
        self.mack     = self.con.master_ack
        self.hold     = self.con.hold
        self.flush    = self.con.reset
        self.pip_wait = self.con.pip_wait_reg


def _build(cls, debug: bool = True):
    reset()
    ran.clear()
    return build_model(cls(), debug=debug)


# ---- the dbg phase is opt-in -------------------------------------------------

def test_build_model_without_debug_runs_no_dbg_body():
    m = _build(two_pips_dbg, debug=False)
    assert ran == [] and not hasattr(m, "pip_wait")


def test_gen_dbg_is_a_dedicated_call_after_build_flow():
    reset()
    ran.clear()
    m = two_pips_dbg()
    set_top(m)
    gen_flow()
    build_flow()
    assert ran == []                                        # build_flow alone runs no @dbg body
    gen_dbg()
    assert ran == [1] and isinstance(m.pip_wait, SignalRef)


# ---- after build_model(debug=True) ------------------------------------------

def test_the_dbg_body_runs_once_and_reads_signal_refs():
    m = _build(two_pips_dbg)
    assert ran == [1]
    for ref in (m.mack, m.hold, m.flush, m.pip_wait):
        assert isinstance(ref, SignalRef), ref
    assert m.hold.global_id  == m.hold_w.global_id
    assert m.flush.global_id == m.flush_w.global_id
    assert m.mack.ident.hw_type == "EXPR"                  # the pip entrance (pseudo node) expression
    assert arena().hcp_verilog_name(m.pip_wait.ident).startswith("SR_ST_pip_wait4syn")


def test_the_pipcon_records_which_leaf_and_block_are_the_pips():
    m = _build(two_pips)
    assert m.con.leaf_count      == 2
    assert m.con.pip_leaf.index  == 1
    assert m.con.pip_leaf_idx    == 1
    assert m.con.pip_block_i.block_type == m.src.pip_block_i.block_type
    assert m.src.pip_leaf.index  == 0
    assert m.sink.pip_leaf is None and m.sink.pip_leaf_idx is None and m.sink.pip_block_i is None
    assert m.sink.pip_wait_reg is None
    assert isinstance(m.sink.master_ack, SignalRef)         # the const 1 of no_pip_master
    assert m.sink.master_ack.hw_type == "VAL"
    assert m.src.hold is None and m.src.reset is None       # never bound on src


def test_a_non_pip_block_has_no_wait_reg():
    m = _build(two_pips)
    with pytest.raises(ValueError, match="not a pipeline"):
        arena().get_pip_wait_reg(m.seq_i)


# ---- before build -------------------------------------------------------------

def test_before_build_the_master_ack_is_unbound_and_the_wait_reg_refuses():
    reset()
    m = two_pips()
    set_top(m)
    gen_flow()
    assert m.con.master_ack is None
    assert m.con.hold.global_id  == m.hold_w.global_id      # bound at set_hold time, in @flow
    assert m.con.reset.global_id == m.flush_w.global_id
    assert m.con.pip_block_i is not None                    # bound at pip() time
    with pytest.raises(ValueError, match="not built"):
        m.con.pip_wait_reg


def test_a_second_pip_on_one_pipcon_is_refused():
    class double(Module):
        @init
        def com_declare(self):
            self.con, self.sink = PipCon(), PipCon()
            self.sink.no_pip_master()
            self.a = reg(8, "a")

        @flow
        def my_flow(self):
            with pip(self.con, auto_req=True):
                with zync(self.sink, auto_ack=True):
                    self.a |= self.a + 1
            with pip(self.con, auto_req=True):
                with zync(self.sink, auto_ack=True):
                    self.a |= self.a + 1

    reset()
    set_top(double())
    with pytest.raises(ValueError, match="one pip per PipCon"):
        gen_flow()


# ---- emit ---------------------------------------------------------------------

EMIT_SCRIPT = textwrap.dedent("""
    import sys
    sys.path.insert(0, sys.argv[2])                          # this test's directory: the model lives here
    from kathryn import build_model, emit_verilog, reset
    from test_pip_dbg_signals import two_pips, two_pips_dbg
    reset()
    model = two_pips_dbg if sys.argv[3] == "dbg" else two_pips
    build_model(model(name="two_pips"), debug=True)          # explicit: a subclass would auto-name itself apart
    emit_verilog(sys.argv[1])
""")


def _emit_in_subprocess(out_dir, variant: str) -> None:
    env = dict(os.environ, PYTHONPATH=os.pathsep.join(p for p in (_py_dir(), os.environ.get("PYTHONPATH", "")) if p))
    subprocess.run([sys.executable, "-c", EMIT_SCRIPT, str(out_dir), os.path.dirname(os.path.abspath(__file__)), variant],
                   check=True, env=env, capture_output=True)


def _py_dir() -> str:
    import kathryn
    return os.path.dirname(os.path.dirname(kathryn.__file__))


def test_the_dbg_method_changes_no_emitted_hardware(tmp_path):
    for variant in ("plain", "dbg"):
        (tmp_path / variant).mkdir()
        _emit_in_subprocess(tmp_path / variant, variant)
    assert filecmp.cmp(tmp_path / "plain" / "top.v", tmp_path / "dbg" / "top.v", shallow=False)


def test_the_manifest_lists_the_dbg_attributes(tmp_path):
    _build(two_pips_dbg)
    emit_verilog(str(tmp_path))
    with open(os.path.join(str(tmp_path), "sim_manifest.json"), encoding="utf-8") as f:
        children = json.load(f)["root"]["children"]
    assert children["pip_wait"]["kind"] == "signal"
    assert children["pip_wait"]["verilog"].startswith("SR_ST_pip_wait4syn")
    for name in ("mack", "hold", "flush"):
        assert children[name]["kind"] == "signal", name

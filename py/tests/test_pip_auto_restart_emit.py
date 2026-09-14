# A flushed `pip(auto_restart=True)` restarts because a SET outranks the RST it
# races: on every state register the flush's UNSET is emitted BEFORE the SET, so
# the SET is the last write and wins. That ladder is the whole mechanism — the
# pip wiring is untouched. What `auto_restart` adds is only the edge that raises
# the entrance during the flush, which is what asserts a SET in that cycle; a
# plain pip has no such edge, so it is cleared and stays dead.
# The cocotb twin is test/model/tc42_pip_auto_restart.py; this test reads the
# emitted Verilog, so it fails first on a stale extension build.

import os
import re

from kathryn import (Module, PipCon, build_model, emit_verilog, flow, init, pip,
                     reg, reset, wire, zif, zync)

FLUSH_AT = 3

# One `if (cond) SR_ST_x <= VAL_..._SET/UNSET_n;` write, in emitted order.
_EVENT = re.compile(r"if \((\w+)\) begin\s*\n\s*(SR_ST_\w+) <= VAL_\w+_(SET|UNSET)_\d+;")
_FLUSH = re.compile(r"WIRE_arb_flush_\d+")


def _one_pip_model(auto_restart: bool):
    class one_pip(Module):
        @init
        def com_declare(self):
            self.con, self.sink           = PipCon(), PipCon()
            self.tick_con, self.tick_sink = PipCon(), PipCon()
            self.tick  = reg(8, "tick")
            self.count = reg(8, "count")
            self.pulse = wire(1, "pulse")

        @flow
        def my_flow(self):
            self.tick.reset(0)
            self.count.reset(0)
            with pip(self.tick_con, auto_req=True):        # always active: the zif pulses one cycle
                with zync(self.tick_sink, auto_ack=True):
                    self.tick |= self.tick + 1
                    with zif(self.tick == FLUSH_AT):
                        self.pulse *= self.con.flush()
            with pip(self.con, auto_req=True, auto_restart=auto_restart):
                with zync(self.sink, auto_ack=True):
                    self.count |= self.count + 1
    return one_pip


def _emit(tmp_path, auto_restart: bool) -> str:
    reset()
    build_model(_one_pip_model(auto_restart)())
    emit_verilog(str(tmp_path))
    with open(os.path.join(str(tmp_path), "top.v"), encoding="utf-8") as f:
        return f.read()


def _events_on(verilog: str, state_reg: str):
    """(condition, SET|UNSET) for one state register, in emitted order."""
    return [(cond, value) for cond, name, value in _EVENT.findall(verilog) if name == state_reg]


def _flushed_state(verilog: str, prefix: str) -> str:
    """The one `prefix` state register a flush wire clears.

    - the design holds two pips and only one is flushed, so this is what anchors
      every other lookup on the pip under test
    """
    hits = {name for cond, name, _ in _EVENT.findall(verilog)
            if name.startswith(prefix) and _FLUSH.fullmatch(cond)}
    assert len(hits) == 1, f"expected one flushed {prefix}*, found {hits}"
    return hits.pop()


def _entrance_of(verilog: str, state_reg: str) -> str:
    """The pip entrance that SETs `state_reg` — `<something> & EXPR_pseudo_init_expr_N`.

    - NEVER search for the first pseudo_init_expr in the file: a design with more
      than one pip emits several and the first is some other pipeline's
    """
    for cond, value in _events_on(verilog, state_reg):
        if value != "SET":
            continue
        found = re.search(rf"assign\s+{cond}\s*=[^;]*?(EXPR_pseudo_init_expr_\d+)", verilog)
        if found:
            return found.group(1)
    raise AssertionError(f"no entrance drives a SET on {state_reg}")


def _entrance_terms(verilog: str, root_expr: str):
    """Every assign `root_expr` is built from, followed through the logic chain."""
    assigns = dict(re.findall(r"assign\s+(EXPR_node_logic_expr_\d+)\s*=\s*([^;]*);", verilog))
    root    = re.search(rf"assign\s+{root_expr}\s*=\s*(EXPR_node_logic_expr_\d+);", verilog)
    assert root is not None, f"no logic chain behind {root_expr}"
    todo, seen = [root.group(1)], []
    while todo:
        name = todo.pop()
        if name in seen or name not in assigns:
            continue
        seen.append(name)
        todo += re.findall(r"EXPR_node_logic_expr_\d+", assigns[name])
    return [assigns[name] for name in seen]


def _feeds_un_negated_flush(verilog: str, root_expr: str) -> bool:
    terms = _entrance_terms(verilog, root_expr)
    assert terms, f"the chain behind {root_expr} is empty"
    return any(re.search(r"(?<!~)WIRE_arb_flush_\d+", term) for term in terms)


# ---- the ladder: a set outranks the reset it races ---------------------------

def test_a_set_is_emitted_after_the_flush_it_races_on_the_wait_register(tmp_path):
    verilog  = _emit(tmp_path, auto_restart=True)
    wait4syn = _flushed_state(verilog, "SR_ST_pip_wait4syn_")
    events   = _events_on(verilog, wait4syn)

    clears = [i for i, (cond, value) in enumerate(events) if value == "UNSET" and _FLUSH.fullmatch(cond)]
    sets   = [i for i, (cond, value) in enumerate(events) if value == "SET"]
    assert clears and sets, events
    assert max(sets) > max(clears), f"the flush UNSET still outranks the SET: {events}"


def test_a_set_is_emitted_after_the_flush_it_races_on_the_sub_block_state(tmp_path):
    # The state that actually carries the work, not just the wait register.
    verilog = _emit(tmp_path, auto_restart=True)
    zync    = _flushed_state(verilog, "SR_ST_zync_state_")
    events  = _events_on(verilog, zync)

    clears = [i for i, (cond, value) in enumerate(events) if value == "UNSET" and _FLUSH.fullmatch(cond)]
    sets   = [i for i, (cond, value) in enumerate(events) if value == "SET"]
    assert clears and sets, events
    assert max(sets) > max(clears), f"the flush UNSET still outranks the SET: {events}"


def test_the_master_reset_still_beats_everything(tmp_path):
    verilog  = _emit(tmp_path, auto_restart=True)
    wait4syn = _flushed_state(verilog, "SR_ST_pip_wait4syn_")
    events   = _events_on(verilog, wait4syn)
    mrst     = [i for i, (cond, _) in enumerate(events) if cond.startswith("WIRE_mrst_")]
    assert mrst, events
    assert max(mrst) == len(events) - 1, f"mrst is not the last write: {events}"


# ---- what auto_restart itself adds: the entrance edge ------------------------

def test_auto_restart_raises_the_entrance_during_the_flush(tmp_path):
    # Without this edge nothing asserts a SET in the flush cycle, and the ladder
    # above would have nothing to rescue.
    verilog  = _emit(tmp_path, auto_restart=True)
    wait4syn = _flushed_state(verilog, "SR_ST_pip_wait4syn_")
    assert _feeds_un_negated_flush(verilog, _entrance_of(verilog, wait4syn))


def test_a_plain_pip_keeps_the_flush_out_of_its_entrance(tmp_path):
    verilog  = _emit(tmp_path, auto_restart=False)
    wait4syn = _flushed_state(verilog, "SR_ST_pip_wait4syn_")
    assert not _feeds_un_negated_flush(verilog, _entrance_of(verilog, wait4syn))

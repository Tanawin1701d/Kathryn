# DebugProbe — a plain Python class the sim manifest walks like a Module: its
# non-underscore attributes become the children of a "probe" node, which the
# reader resolves in the scope of the Module that holds it (no hierarchy hop).

import os
from types import SimpleNamespace

import pytest

from kathryn import (DebugProbe, Module, arena, build_model, dbg, emit_verilog, flow, init, reg, reset,
                     seq, wire)
from kathryn.sim.ksim import KSim
from kathryn.sim.manifest import CHILDREN_KEY_OF, NODE_KINDS
from kathryn.sim.manifest.write import VerilogSimNamer, harvest_sim_tree


class stage_probe(DebugProbe):
    # A subclass with its own fields walks the same as a kwargs-built probe.
    def __init__(self, head, tail):
        self.head = head
        self.tail = tail


class probed(Module):
    @init
    def com_declare(self):
        self.a = reg (8, "a")
        self.b = wire(8, "b")
        self.c = reg (8, "c")

    @flow
    def my_flow(self):
        self.b *= self.a + 1
        with seq():
            self.a |= 3
            self.c |= self.b

    @dbg
    def probe(self):
        self.pipe         = DebugProbe(head=self.a, hi=self.a[7, 4], nested=stage_probe(self.a, self.c))
        self.pipe._hidden = self.b                 # `_` attribute: skipped, like on a Module
        self.pipe.note    = "not hardware"         # dropped, like on a Module


def _build():
    reset()
    return build_model(probed(), debug=True)


def _harvest(m):
    return harvest_sim_tree(m, VerilogSimNamer())["root"]["children"]


# ---- writer -------------------------------------------------------------------

def test_the_probe_kind_is_in_the_shared_schema():
    assert "probe" in NODE_KINDS and CHILDREN_KEY_OF["probe"] == "children"


def test_a_probe_is_a_module_shaped_node_with_no_instance():
    m    = _build()
    node = _harvest(m)["pipe"]
    assert node["kind"] == "probe" and "instance" not in node
    kids = node["children"]
    assert sorted(kids) == ["head", "hi", "nested"]          # `_hidden` skipped, `note` dropped
    assert kids["head"]["kind"] == "signal" and kids["head"]["verilog"] == arena().hcp_verilog_name(m.a.ident)
    assert kids["hi"]["kind"]   == "slice"  and (kids["hi"]["msb"], kids["hi"]["lsb"]) == (7, 4)
    assert kids["nested"]["kind"] == "probe"
    assert sorted(kids["nested"]["children"]) == ["head", "tail"]
    assert kids["nested"]["children"]["tail"]["verilog"] == arena().hcp_verilog_name(m.c.ident)


def test_the_same_probe_under_two_attributes_is_two_entries():
    m       = _build()
    m.again = m.pipe
    kids    = _harvest(m)
    assert kids["again"] == kids["pipe"]


def test_a_probe_cycle_is_refused():
    m = _build()
    m.pipe.nested.back = m.pipe
    with pytest.raises(ValueError, match="attribute cycle"):
        _harvest(m)


# ---- reader -------------------------------------------------------------------

def test_the_reader_resolves_a_probe_in_the_holding_scope(tmp_path):
    m     = _build()
    names = {attr: arena().hcp_verilog_name(getattr(m, attr).ident) for attr in ("a", "b", "c")}
    emit_verilog(str(tmp_path))                                # moves the arena: names were read first
    dut = SimpleNamespace(**{name: SimpleNamespace(value=0) for name in names.values()})
    k   = KSim(dut, os.path.join(str(tmp_path), "sim_manifest.json"))
    assert k.pipe.head is getattr(dut, names["a"])            # the top handle itself: no getattr hop
    assert k.pipe.head is k.a                                  # the same raw handle as the Module attribute
    assert k.pipe.nested.tail is getattr(dut, names["c"])
    assert dir(k.pipe) == ["head", "hi", "nested"]
    getattr(dut, names["a"]).value = 0xA5
    assert k.pipe.hi.value == 0xA
    with pytest.raises(AttributeError, match="DebugProbe"):
        k.pipe.note

# The manifest as a tree — paths of names and [indexes] resolve to nodes, a
# wrong hop names what IS there, and the same path walks a live object tree.

from __future__ import annotations

from types import SimpleNamespace

import pytest

from kathryn.sim.manifest import ManifestError, Manifest, child_node, parse_path, walk_path

MANIFEST = {
    "schema_version": 1, "backend": "verilog", "top_module": "MODULE_Top0_1",
    "root": {"kind": "module", "instance": None, "children": {
        "core": {"kind": "module", "instance": "MODULE_Core0_2", "children": {
            "rob"  : {"kind": "dict", "entries": {
                "table": {"kind": "karray", "shape": [4], "fields": [["valid", 1], ["pc", 32]], "elements": []}}},
            "lanes": {"kind": "list", "items": [{"kind": "signal", "verilog": "REG_a_3", "width": 8}]},
            "cnt"  : {"kind": "counter", "value": "REG_c_4", "now": "WIRE_c_5", "width": 8},
            "hi"   : {"kind": "slice", "verilog": "REG_a_3", "msb": 7, "lsb": 4}}}}}}


def test_a_path_is_names_and_indexes():
    assert parse_path("a.b[2].c") == ("a", "b", 2, "c")
    assert parse_path("x[0][1]") == ("x", 0, 1)
    for bad in ("", "a..b", "a[x]", "a b"):
        with pytest.raises(ManifestError):
            parse_path(bad)


def test_nodes_resolve_and_modules_along_the_way_are_named():
    tree = Manifest(MANIFEST)
    assert tree.top_module == "MODULE_Top0_1"
    assert tree.node("core.rob.table")["kind"] == "karray"
    assert tree.node("core.lanes[0]")["width"] == 8
    assert tree.modules_along("core.rob.table") == ["MODULE_Core0_2"]
    assert tree.emitted_name(tree.node("core.lanes[0]")) == "REG_a_3"
    assert tree.emitted_name(tree.node("core.cnt")) == "REG_c_4"
    assert tree.emitted_name(tree.node("core.hi")) == "REG_a_3"
    assert tree.karray_shape(tree.node("core.rob.table")) == ([4], ["valid", "pc"], [1, 32])


def test_a_wrong_hop_says_what_is_there():
    tree = Manifest(MANIFEST)
    with pytest.raises(ManifestError, match="available: \\['cnt', 'hi', 'lanes', 'rob'\\]"):
        tree.node("core.nope")
    with pytest.raises(ManifestError, match="indexes a module node"):
        tree.node("core[0]")
    with pytest.raises(ManifestError, match="outside the list's 1 items"):
        tree.node("core.lanes[3]")
    with pytest.raises(ManifestError, match="has no children"):
        tree.node("core.cnt.value")
    with pytest.raises(ManifestError, match="no single emitted name"):
        tree.emitted_name(tree.node("core.rob.table"))
    with pytest.raises(ManifestError, match="expected a karray"):
        tree.karray_shape(tree.node("core.cnt"))
    with pytest.raises(ManifestError):
        child_node({"kind": "signal"}, "x", "x")


def test_the_same_path_walks_a_live_object_tree():
    live = SimpleNamespace(core=SimpleNamespace(lanes=[SimpleNamespace(pc=7)], rob={"table": 3}))
    assert walk_path(live, "core.lanes[0].pc") == 7
    assert walk_path(live, "core.rob")["table"] == 3

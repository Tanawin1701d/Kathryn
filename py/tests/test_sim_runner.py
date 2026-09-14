# The cached simulator build — what the fingerprint sees, what a junit file
# says, and the fact the cache rests on: two emits of one design in two
# processes are byte-identical.

from __future__ import annotations

import filecmp
import json
import os
import subprocess
import sys
import textwrap

from kathryn.sim.rtl import VerilogRtl, open_rtl
from kathryn.sim.runner_cocotb import cache_key, read_test_status

TOP_A = "module MODULE_Top0_1 (\n  input clk,\n  output IO_WIRE_IO_OUT_z_9,\n  input IO_WIRE_IO_IN_q_4\n);\nreg WIRE_x_3;\nendmodule\n"
TOP_B = "module MODULE_Top0_1 (\n  input clk,\n  input IO_WIRE_IO_IN_q_12,\n  output IO_WIRE_IO_OUT_z_7\n);\nreg WIRE_x_3;\nendmodule\n"


class _Backend:
    name = "fake"
    def build_args(self):           return ["-x"]
    def waves_file(self, top):      return "dump.vcd"
    def describe(self):             return "fake 1.0"
    def make_runner(self):          raise AssertionError("never built")


def test_normalising_takes_the_port_ids_and_their_order_out():
    normalize = VerilogRtl.normalize
    assert normalize(TOP_A) == normalize(TOP_B)
    assert normalize(normalize(TOP_A).decode()) == normalize(TOP_A)
    assert b"IO_WIRE_IO_OUT_z_9" not in normalize(TOP_A)


def test_the_fingerprint_is_the_same_for_the_two_textual_variants(tmp_path):
    for name, text in (("a", TOP_A), ("b", TOP_B)):
        (tmp_path / name).mkdir()
        (tmp_path / name / "top.v").write_text(text)
        (tmp_path / name / "sim_manifest.json").write_text(json.dumps({"top_module": "MODULE_Top0_1", "root": {}}))
    digest = lambda name: open_rtl(tmp_path / name).sources_digest().hexdigest()
    key    = lambda name: cache_key(open_rtl(tmp_path / name), _Backend())
    assert digest("a") == digest("b") and key("a") == key("b")
    (tmp_path / "a" / "top.v").write_text(TOP_A.replace("WIRE_x_3", "WIRE_x_4"))
    assert digest("a") != digest("b") and key("a") != key("b")


def test_the_top_module_and_the_language_come_from_the_manifest(tmp_path):
    (tmp_path / "sim_manifest.json").write_text(json.dumps({"top_module": "MODULE_Top0_1", "root": {}}))
    rtl = open_rtl(tmp_path)
    assert rtl.top_module == "MODULE_Top0_1"
    assert isinstance(rtl, VerilogRtl)              # no backend tag -> the verilog reader, like the other readers


def test_a_junit_file_says_pass_fail_or_nothing(tmp_path):
    ok = tmp_path / "ok.xml"
    ok.write_text('<testsuites><testsuite><testcase name="t"/></testsuite></testsuites>')
    bad = tmp_path / "bad.xml"
    bad.write_text('<testsuites><testsuite><testcase name="t"><failure message="no"/></testcase></testsuite></testsuites>')
    empty = tmp_path / "empty.xml"
    empty.write_text("<testsuites/>")
    broken = tmp_path / "broken.xml"
    broken.write_text("<testsuites")
    assert read_test_status(ok) == "PASS" and read_test_status(bad) == "FAIL"
    assert read_test_status(empty) == "NO_RESULT" and read_test_status(broken) == "NO_RESULT"
    assert read_test_status(tmp_path / "missing.xml") == "NO_RESULT"


EMIT_SCRIPT = textwrap.dedent("""
    import sys
    from kathryn import Module, build_model, emit_verilog, flow, init, reg, reset, seq, wire

    class Child(Module):
        @init
        def decl(self):
            self.acc = reg(8, "acc")
        @flow
        def my_flow(self):
            with seq():
                self.acc |= self.acc + 1

    class Parent(Module):
        @init
        def decl(self):
            self.kids = [Child() for _ in range(3)]
            self.total = wire(8, "total")
        @flow
        def my_flow(self):
            self.total *= self.kids[0].acc + self.kids[1].acc + self.kids[2].acc

    reset()
    build_model(Parent())
    emit_verilog(sys.argv[1])
""")


def test_two_emits_of_one_design_in_two_processes_are_byte_identical(tmp_path):
    for name in ("a", "b"):
        (tmp_path / name).mkdir()
        env = dict(os.environ, PYTHONPATH=os.pathsep.join(p for p in (str(_py_dir()), os.environ.get("PYTHONPATH", "")) if p))
        subprocess.run([sys.executable, "-c", EMIT_SCRIPT, str(tmp_path / name)], check=True, env=env, capture_output=True)
    files = sorted(p.name for p in (tmp_path / "a").iterdir())
    assert "top.v" in files and any("Child" in name for name in files)
    for name in files:
        assert filecmp.cmp(tmp_path / "a" / name, tmp_path / "b" / name, shallow=False), name


def _py_dir():
    import kathryn
    return os.path.dirname(os.path.dirname(kathryn.__file__))

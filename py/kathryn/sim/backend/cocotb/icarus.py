# Sim backend ICARUS — the one-line switch away from Verilator.
# - Icarus dumps a plain VCD; cocotb's stock runner would write FST.

from __future__ import annotations

import shutil
from typing import List

from ..base import read_version_line
from .base import CocotbBackend


class IcarusBackend(CocotbBackend):
    name = "icarus"

    def build_args(self) -> List[str]:          return []
    def waves_file(self, toplevel: str) -> str: return f"{toplevel}.vcd"
    def describe(self) -> str:                  return read_version_line(shutil.which("iverilog"))
    def make_runner(self):                      return make_icarus_runner()


def make_icarus_runner():
    """cocotb's Icarus runner, writing a plain VCD where the stock one writes FST.

    - three FST-specific spots are overridden: the dump-file name, the
      generated $dumpfile module, and the `-fst` vvp plusarg
    """
    from cocotb_tools.runner import Icarus, _as_sv_literal

    class IcarusVcd(Icarus):
        def _waves_file(self):
            return f"{self.hdl_toplevel}.vcd"

        def _create_iverilog_dump_file(self):
            dumpfile = _as_sv_literal(str(self.build_dir / f"{self.hdl_toplevel}.vcd"))
            with open(self.iverilog_dump_file, "w") as f:
                f.write("module cocotb_iverilog_dump();\n"
                        "initial begin\n"
                        f"    $dumpfile({dumpfile});\n"
                        f"    $dumpvars(0, {self.hdl_toplevel});\n"
                        "end\n"
                        "endmodule\n")

        def _test_command(self):
            cmds = super()._test_command()
            for cmd in cmds:
                while "-fst" in cmd:
                    cmd.remove("-fst")
            return cmds

    return IcarusVcd()


def icarus_is_available() -> bool:
    return shutil.which("iverilog") is not None

# Icarus (iverilog) backend + its VCD-emitting runner subclass.
#
# cocotb 2.x's stock Icarus runner dumps FST when waves=True. We want a plain
# VCD, so `IcarusVCD` overrides the three FST-specific spots: the dump-file path,
# the generated $dumpfile module, and the `-fst` vvp plusarg.

from __future__ import annotations

from .backend import SimBackend

try:
    from cocotb_tools.runner import Icarus as _Icarus, _as_sv_literal

    class IcarusVCD(_Icarus):
        def _waves_file(self):
            return f"{self.hdl_toplevel}.vcd"

        def _create_iverilog_dump_file(self):
            dumpfile = _as_sv_literal(str(self.build_dir / f"{self.hdl_toplevel}.vcd"))
            with open(self.iverilog_dump_file, "w") as f:
                f.write("module cocotb_iverilog_dump();\n")
                f.write("initial begin\n")
                f.write(f"    $dumpfile({dumpfile});\n")
                f.write(f"    $dumpvars(0, {self.hdl_toplevel});\n")
                f.write("end\n")
                f.write("endmodule\n")

        def _test_command(self):
            # vvp emits VCD by default (per $dumpfile extension); drop the FST flag.
            cmds = super()._test_command()
            for cmd in cmds:
                while "-fst" in cmd:
                    cmd.remove("-fst")
            return cmds
except ImportError:   # cocotb not installed / older API — only matters at run time
    IcarusVCD = None


class IcarusBackend(SimBackend):
    def __init__(self):
        super().__init__("icarus")

    def make_runner(self):
        if IcarusVCD is None:
            raise RuntimeError("cocotb_tools.runner.Icarus unavailable — is cocotb installed?")
        return IcarusVCD()

    # waves_file inherited from SimBackend → "<toplevel>.vcd" (matches IcarusVCD._waves_file).

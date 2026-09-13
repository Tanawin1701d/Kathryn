# cocotb backend BASE — drives cocotb's runner for one tool.
# - A subclass makes cocotb's runner for its tool (make_runner); compile() and
#   run() here drive that runner the same way for every tool.
# - cocotb is imported only inside a subclass's make_runner(), so this package
#   imports with no simulator installed and availability can be asked first.
# - How cocotb spells each emitted LANGUAGE (COCOTB_HDL_OF) is here: it is
#   cocotb's vocabulary, so it belongs to the harness and not to Rtl.

from __future__ import annotations

import pathlib
from abc import abstractmethod
from typing import Mapping, Optional, Tuple

from ...rtl import Rtl
from ..base import SimBackend

TIMESCALE = ("1ns", "1ps")

# manifest backend tag -> (hdl_toplevel_lang, the runner.build() keyword its sources go under)
COCOTB_HDL_OF = {"verilog": ("verilog", "verilog_sources")}


def cocotb_hdl_of(rtl: Rtl) -> Tuple[str, str]:
    if rtl.backend_tag not in COCOTB_HDL_OF:
        raise ValueError(f"cocotb has no spelling for a '{rtl.backend_tag}' emit — one of {sorted(COCOTB_HDL_OF)}")
    return COCOTB_HDL_OF[rtl.backend_tag]


class CocotbBackend(SimBackend):
    """One simulator as cocotb's runner drives it."""

    @abstractmethod
    def make_runner(self): ...                      # a cocotb_tools.runner for this tool

    # ---- driving the runner: the same for every tool ---------------------------

    def compile(
        self,
        rtl       : Rtl,
        build_dir : pathlib.Path,
        waves     : bool = False,
        log_path  : Optional[pathlib.Path] = None,
    ) -> None:
        """cocotb's runner.build() for this tool, into build_dir."""
        _, sources_arg = cocotb_hdl_of(rtl)
        self.make_runner().build(**{sources_arg: [str(path) for path in rtl.sources()]},
                                 hdl_toplevel = rtl.top_module,
                                 build_dir    = str(build_dir),
                                 build_args   = self.build_args(),
                                 always       = True,
                                 waves        = waves,
                                 timescale    = TIMESCALE,
                                 log_file     = str(log_path) if log_path else None)

    def run(
        self,
        rtl         : Rtl,
        build_dir   : pathlib.Path,
        test_module : str,
        testcase    : str,
        results_xml : pathlib.Path,
        env         : Mapping[str, str],
        waves       : bool = False,
        log_path    : Optional[pathlib.Path] = None,
    ) -> None:
        """cocotb's runner.test() for one testcase; the verdict lands in results_xml.

        - a failing test makes the runner exit non-zero; that is swallowed here,
          the junit file says why
        """
        lang, _ = cocotb_hdl_of(rtl)
        try:
            self.make_runner().test(test_module       = test_module,
                                    testcase          = testcase,
                                    hdl_toplevel      = rtl.top_module,
                                    hdl_toplevel_lang = lang,       # a fresh runner never saw build(): say it
                                    build_dir         = str(build_dir),
                                    results_xml       = str(results_xml),
                                    extra_env         = dict(env),
                                    waves             = waves,
                                    timescale         = TIMESCALE,
                                    log_file          = str(log_path) if log_path else None)
        except SystemExit:
            pass


def cocotb_is_available() -> bool:
    try:
        import cocotb  # noqa: F401
        return True
    except ImportError:
        return False

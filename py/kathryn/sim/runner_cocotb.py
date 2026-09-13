# Sim RUNNER — compile an emitted design once, then run cocotb tests against
# that build.
# - The compiled simulator is cached under <cache_root>/<cache_key>/: the Rtl's
#   sources digest plus what THIS harness's build depends on (backend, flags,
#   tool version, waves), so a run on the same design never recompiles and a
#   changed design or an upgraded tool does.
# - A CocotbSim carries the backend that built it: the same one must run
#   it, so a build can never be paired with the wrong simulator.
# - The manifest is not part of the build: a run reads it from the rtl dir the
#   fingerprint was taken from, and the two must come from the same emit.
# - CocotbSim.run_test passes the manifest path to the simulator process through
#   $KATHRYN_SIM_MANIFEST, so KSim(dut) in the test needs no argument.

from __future__ import annotations

import json
import os
import pathlib
import time
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from typing import Mapping, Optional

from .backend import SimBackend
from .backend.cocotb import CocotbBackend
from .ksim import SIM_MANIFEST_ENV
from .manifest import SIM_MANIFEST_FILE
from .rtl import Rtl

OK_MARKER = "ok"                        # written after a build that finished


def cache_key(rtl: Rtl, backend: SimBackend, waves: bool = False) -> str:
    """16 hex digits over the sources digest and everything this build depends on."""
    digest = rtl.sources_digest()
    facts  = {"backend": backend.name,
              "args"   : backend.build_args(),
              "waves"  : waves,
              "tool"   : backend.describe()}
    digest.update(json.dumps(facts, sort_keys=True).encode())
    return digest.hexdigest()[:16]


@dataclass(frozen=True)
class CocotbSim:
    """One simulator compiled by cocotb's runner; run_test is the only way to run it.

    - carries the emit it was built from and the backend that built it
    """

    rtl       : Rtl                     # its manifest goes to the run
    backend   : CocotbBackend           # the one that built it is the one that runs it
    build_dir : pathlib.Path
    reused    : bool                    # True when the cache already held it
    seconds   : float                   # 0.0 when reused

    @property
    def top_module(self) -> str: return self.rtl.top_module

    # ---- build ---------------------------------------------------------------

    @classmethod
    def build(
        cls,
        rtl        : Rtl,
        backend    : CocotbBackend,
        cache_root : pathlib.Path,
        waves      : bool = False,
        log_path   : Optional[pathlib.Path] = None,
    ) -> "CocotbSim":
        """The compiled simulator for this emit, built only when not cached."""
        build_dir = pathlib.Path(cache_root) / cache_key(rtl, backend, waves)
        if (build_dir / OK_MARKER).is_file():
            return cls(rtl, backend, build_dir, reused=True, seconds=0.0)

        started = time.perf_counter()
        backend.compile(rtl, build_dir, waves=waves, log_path=log_path)
        (build_dir / OK_MARKER).write_text(f"{rtl.top_module}\n", encoding="utf-8")
        return cls(rtl, backend, build_dir, reused=False, seconds=time.perf_counter() - started)

    # ---- run -----------------------------------------------------------------

    def run_test(
        self,
        test_module : str,
        testcase    : str,
        extra_env   : Mapping[str, str] = (),
        waves       : bool = False,
        log_path    : Optional[pathlib.Path] = None,
    ) -> str:
        """Run one @cocotb.test against this build; PASS, FAIL or NO_RESULT.

        - the manifest of the build's rtl dir reaches the simulator process as
          $KATHRYN_SIM_MANIFEST; `extra_env` may add to that
        """
        results = self.build_dir / f"{testcase}.results.xml"
        results.unlink(missing_ok=True)
        env = {SIM_MANIFEST_ENV: str(self.rtl.dir / SIM_MANIFEST_FILE), **dict(extra_env)}
        os.environ.update(env)              # the runner lets os.environ win over extra_env
        self.backend.run(self.rtl, self.build_dir, test_module, testcase, results, env,
                         waves=waves, log_path=log_path)
        return read_test_status(results)


# ---- results -----------------------------------------------------------------

def read_test_status(results_xml: pathlib.Path) -> str:
    """PASS / FAIL / NO_RESULT from cocotb's junit file; NO_RESULT when it is missing or unreadable."""
    if not results_xml.is_file():
        return "NO_RESULT"
    try:
        cases = list(ET.parse(results_xml).getroot().iter("testcase"))
    except ET.ParseError:
        return "NO_RESULT"
    if not cases:
        return "NO_RESULT"
    failed = any(case.find("failure") is not None or case.find("error") is not None
                 for case in cases)
    return "FAIL" if failed else "PASS"

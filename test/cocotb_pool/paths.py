# Path constants + the mutable pool configuration.
#
# `configure()` may repoint the pool at another project's test tree, so the
# mutable values (MODEL / OUT_ROOT / EXTRA_PYTHONPATH) live here and are always
# referenced *qualified* (`paths.MODEL`) by the rest of the package — never
# `from .paths import MODEL`, which would snapshot a stale binding.

from __future__ import annotations

import pathlib

# ---- fixed paths -------------------------------------------------------------
HERE    = pathlib.Path(__file__).resolve().parent.parent    # .../Kathryn2/test
REPO    = HERE.parent                                        # .../Kathryn2
PY_DIR  = REPO / "py"                                        # kathryn package

# ---- mutable configuration (see configure()) ---------------------------------
MODEL    = HERE / "model"                                    # tc*.py live here
OUT_ROOT = HERE / ".model_output"                            # per-case build dirs

# Extra dirs other projects need on PYTHONPATH inside the sim subprocess
# (set via configure(); empty for Kathryn's own tests).
EXTRA_PYTHONPATH: list[str] = []


def configure(model_dir=None, out_root=None, extra_pythonpath=()) -> None:
    # Point the pool at ANOTHER project's test tree so downstream projects reuse
    # this runner unchanged. Every argument is optional; Kathryn's own defaults
    # stay intact when it is never called.
    global MODEL, OUT_ROOT, EXTRA_PYTHONPATH
    if model_dir is not None:
        MODEL = pathlib.Path(model_dir).resolve()
    if out_root is not None:
        OUT_ROOT = pathlib.Path(out_root).resolve()
    EXTRA_PYTHONPATH = [str(pathlib.Path(p).resolve()) for p in extra_pythonpath]

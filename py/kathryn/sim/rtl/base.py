# Sim RTL BASE — one emitted design as a simulator sees it: where its files
# are, what its top is, and how its text is hashed for the build cache.
# - One subclass per GENERATED LANGUAGE (rtl/verilog.py today); open_rtl()
#   picks it from the manifest's "backend" tag, the way SIM_NAMERS does.
# - The language hooks are needed BEFORE a simulator exists — the fingerprint
#   decides whether to build at all — which is why they live on the emit and
#   not on CocotbSim.
# - NOT here: anything a harness calls the language (cocotb's `verilog_sources=`).
#   An Rtl knows no simulator; the harness maps `backend_tag` to its own words.

from __future__ import annotations

import hashlib
import json
import pathlib
from typing import Any, Dict, List

from ..manifest import SIM_MANIFEST_FILE


def read_manifest(rtl_dir: pathlib.Path) -> Dict[str, Any]:
    with open(pathlib.Path(rtl_dir) / SIM_MANIFEST_FILE, encoding="utf-8") as f:
        return json.load(f)


class Rtl:
    """One emitted design: its dir, its top module, a digest of its sources, and the
    language hooks below."""

    backend_tag : str = ""              # the manifest "backend" value this class reads

    def __init__(self, rtl_dir: pathlib.Path) -> None:
        manifest = read_manifest(rtl_dir)
        tag      = manifest.get("backend", "verilog")
        if tag != self.backend_tag:
            raise ValueError(f"{type(self).__name__} reads '{self.backend_tag}' emits; this one is '{tag}'")
        self.dir        = pathlib.Path(rtl_dir)
        self.top_module = manifest["top_module"]

    # ---- per-language hooks --------------------------------------------------

    def sources(self) -> List[pathlib.Path]:
        """The files the simulator compiles, in a fixed order."""
        raise NotImplementedError

    @staticmethod
    def normalize(text: str) -> bytes:
        """What the fingerprint hashes for one source file."""
        raise NotImplementedError

    # ---- shared --------------------------------------------------------------

    def sources_digest(self) -> "hashlib._Hash":
        """A sha256 over the normalised source TEXT of every file, and nothing else.

        - a harness appends what ITS build depends on (tool, flags) and takes the
          hexdigest; the language side never knows those facts
        """
        digest = hashlib.sha256()
        for path in self.sources():
            digest.update(path.name.encode())
            digest.update(self.normalize(path.read_text(encoding="utf-8", errors="replace")))
        return digest

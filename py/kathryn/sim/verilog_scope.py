# Sim Verilog SCOPE — where a net is declared, read from the emitted .v files.
# - Hardware built while a @flow runs is declared in the module whose flow is
#   running, which need not be the module whose method built it: an arb leaf's
#   req/ack wires belong to the REQUESTER. The manifest cannot say where such
#   a net is, but the .v can: exactly ONE module declares the plain name
#   (`reg WIRE_x_12;`); every other module that uses it has only an
#   IO_WIRE_IO_IN/OUT_<name>_<id> port.
# - Kathryn emits combinational wires as `reg` driven from always @(*), so
#   both keywords are indexed.
# - Stdlib-only, like the manifest tree beside it.
#
# TODO: DELETE in the next release.  This module answers "which module declares
# a net" by regex-scanning emitted TEXT.  The arena already knows that exactly
# (every HCP carries master_module_i) and can write it into the manifest at
# emit time; SignalProbe / LeafProbe / NamedRegProbe then resolve off the
# manifest like the other three kinds do, and the .v is never parsed.
# Until then: build nothing new on VerilogScope and extend no regex here.
# The reasoning and the migration plan are in CLAUDE.md.

from __future__ import annotations

import pathlib
import re
from dataclasses import dataclass
from typing import Dict, List, Optional

from .manifest import ManifestError, Manifest

MODULE_RE   = re.compile(r"^\s*module\s+(\w+)")
INSTANCE_RE = re.compile(r"^\s*(MODULE_\w+)\s+(MODULE_\w+)\s*\(")
DECL_RE     = re.compile(r"^\s*(?:reg|wire)\s*(?:\[\s*(\d+)\s*:\s*(\d+)\s*\])?\s*(\w+)\s*(;|=|\[)")
PORT_PREFIX = "IO_WIRE_"


class LocateError(ValueError):
    """A name no module declares, or more than one does."""


@dataclass(frozen=True)
class Declaration:
    """One `reg`/`wire` line of one module."""

    name      : str
    module    : str
    width     : int
    is_memory : bool                      # `reg [7:0] NAME [0:N];`


# ---- the scope ---------------------------------------------------------------

class VerilogScope:
    """The instance tree and the declaration index of one emitted rtl dir.

    - TODO: scheduled for DELETION next release; see the module header
    """

    def __init__(
        self,
        parent_of : Dict[str, Optional[str]],
        declared  : Dict[str, List[Declaration]],
    ) -> None:
        self.parent_of = parent_of
        self.declared  = declared
        tops           = [name for name, parent in parent_of.items() if parent is None]
        if len(tops) != 1:
            raise LocateError(f"expected one top module, found {tops}")
        self.top = tops[0]

    # ---- scanning ------------------------------------------------------------

    @classmethod
    def scan(cls, rtl_dir: str) -> "VerilogScope":
        parent_of : Dict[str, Optional[str]] = {}
        declared  : Dict[str, List[Declaration]] = {}
        for path in sorted(pathlib.Path(rtl_dir).glob("*.v")):
            cls._scan_file(path, parent_of, declared)
        return cls(parent_of, declared)

    @staticmethod
    def _scan_file(
        path      : pathlib.Path,
        parent_of : Dict[str, Optional[str]],
        declared  : Dict[str, List[Declaration]],
    ) -> None:
        module = None
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            found = MODULE_RE.match(line)
            if found:
                module = found.group(1)
                parent_of.setdefault(module, None)
                continue
            if module is None:
                continue
            found = INSTANCE_RE.match(line)
            if found:                                     # instance name == module name
                parent_of[found.group(2)] = module
                continue
            found = DECL_RE.match(line)
            if found and not found.group(3).startswith(PORT_PREFIX):
                msb, lsb, name, after = found.groups()
                width = int(msb) - int(lsb) + 1 if msb is not None else 1
                declared.setdefault(name, []).append(
                    Declaration(name, module, width, is_memory=(after == "[")))

    # ---- lookups -------------------------------------------------------------

    def scope_path(self, module: str) -> List[str]:
        """Instance names from just below the top down to `module`; [] for the top."""
        if module not in self.parent_of:
            raise LocateError(f"no module '{module}' in the emitted rtl")
        names = []
        while module != self.top:
            names.append(module)
            module = self.parent_of[module]
        return names[::-1]

    def locate(self, name: str) -> Declaration:
        """The one declaration of `name`."""
        found = self.declared.get(name, [])
        if not found:
            raise LocateError(f"'{name}' is declared in no module (elided, or never a net)")
        if len(found) > 1:
            raise LocateError(f"'{name}' is declared in {[decl.module for decl in found]}, expected one")
        return found[0]

    def find_by_pattern(self, module: str, pattern: str) -> List[Declaration]:
        """Every declaration of `module` matching `pattern`, ids ascending."""
        regex = re.compile(pattern)
        hits  = [decl for decls in self.declared.values() for decl in decls
                 if decl.module == module and regex.search(decl.name)]
        return sorted(hits, key=lambda decl: [int(n) for n in re.findall(r"\d+", decl.name)])

    def find_anywhere(self, pattern: str) -> List[Declaration]:
        """Every declaration in the whole rtl matching `pattern`, ids ascending."""
        regex = re.compile(pattern)
        hits  = [decl for decls in self.declared.values() for decl in decls if regex.search(decl.name)]
        return sorted(hits, key=lambda decl: [int(n) for n in re.findall(r"\d+", decl.name)])


def find_home_module(home_path: str, tree: Manifest, scope: VerilogScope) -> str:
    """The emitted name of the module at a manifest path ("" is the top)."""
    if not home_path:
        return scope.top
    modules = tree.modules_along(home_path)
    if not modules or tree.node(home_path)["kind"] != "module":
        raise ManifestError(f"'{home_path}' is not a module")
    return modules[-1]

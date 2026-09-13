# Manifest READ — the build-side reader of sim_manifest.json: nodes
# addressed by the attribute paths the model uses ("core.rob.table",
# "core.issue_lanes[0][1][0].stage_srcs[1]").
# - Counterpart of ../ksim.py's KSim, which resolves the same paths to live
#   handles inside the simulator; this side reads the FILE in the build
#   process and answers shape and name questions about it.  write.py makes it.
# - Stdlib-only, so a probe declaration can import it with no arena.
# - Node kinds (schema_version 1): module {instance, children}, signal
#   {<backend>, hw_type, width, clocked, depth for a mem_blk}, slice
#   {<backend>, msb, lsb}, counter {value, now, width}, karray {shape,
#   fields, elements}, list {items}, dict {entries}.

from __future__ import annotations

import json
import re
from typing import Any, Dict, List, Tuple, Union

from .schema import CHILDREN_KEY_OF, NODE_KINDS, SIM_MANIFEST_FILE

Token = Union[str, int]
Node  = Dict[str, Any]

_PATH_RE  = re.compile(r"^\w+(?:\.\w+|\[\d+\])*$")     # names joined by dots, [indexes] anywhere
_TOKEN_RE = re.compile(r"(\w+)|\[(\d+)\]")


class ManifestError(ValueError):
    """A path the manifest cannot follow."""


# ---- the manifest ------------------------------------------------------------

class Manifest:
    """The manifest as a tree, with the model's own paths as addresses."""

    def __init__(self, manifest: Dict[str, Any]) -> None:
        self.manifest   = manifest
        self.top_module = manifest["top_module"]
        self.name_key   = manifest.get("backend", "verilog")   # the key an emitted name is under
        self.root       = manifest["root"]

    @classmethod
    def load(cls, path: str) -> "Manifest":
        with open(path, "r", encoding="utf-8") as f:
            return cls(json.load(f))

    @classmethod
    def load_from_rtl_dir(cls, rtl_dir: str) -> "Manifest":
        import os
        return cls.load(os.path.join(rtl_dir, SIM_MANIFEST_FILE))

    # ---- lookups -------------------------------------------------------------

    def node(self, path: str) -> Node:
        node = self.root
        for token in parse_path(path):
            node = child_node(node, token, path)
        return node

    def modules_along(self, path: str) -> List[str]:
        """The instance names of every module hop below the top, in order."""
        node, names = self.root, []
        for token in parse_path(path):
            node = child_node(node, token, path)
            if node["kind"] == "module":
                names.append(node["instance"])
        return names

    def emitted_name(self, node: Node) -> str:
        """The name the backend emitted for a signal, slice or counter node."""
        kind = node["kind"]
        if kind in ("signal", "slice"):
            return node[self.name_key]
        if kind == "counter":
            return node["value"]
        raise ManifestError(f"a {kind} node has no single emitted name")

    @staticmethod
    def karray_shape(node: Node) -> Tuple[List[int], List[str], List[int]]:
        """(shape, field names, field widths) of a karray node."""
        if node["kind"] != "karray":
            raise ManifestError(f"expected a karray node, got {node['kind']}")
        fields = node["fields"]
        return list(node["shape"]), [name for name, _ in fields], [width for _, width in fields]


# ---- paths -------------------------------------------------------------------

def parse_path(path: str) -> Tuple[Token, ...]:
    """"a.b[2].c" -> ("a", "b", 2, "c")."""
    if not _PATH_RE.match(path):
        raise ManifestError(f"'{path}' is not a path of names and [indexes]")
    return tuple(int(index) if index else name for name, index in _TOKEN_RE.findall(path))


def walk_path(obj: Any, path: str) -> Any:
    """Follow "a.b[2].c" through attributes and indexes of any Python object.

    - the live model in the build process, or a KSim tree in the simulator
    """
    for token in parse_path(path):
        obj = obj[token] if isinstance(token, int) else getattr(obj, token)
    return obj


def child_node(node: Node, token: Token, path: str) -> Node:
    """The child one token below `node`; `path` names the whole path in errors."""
    kind = node["kind"]
    if kind not in NODE_KINDS:
        raise ManifestError(f"'{path}': unknown node kind {kind!r} (reader older than writer?)")
    if isinstance(token, int):
        if kind != "list":
            raise ManifestError(f"'{path}': [{token}] indexes a {kind} node, only a list has items")
        items = node[CHILDREN_KEY_OF[kind]]
        if not 0 <= token < len(items):
            raise ManifestError(f"'{path}': index {token} is outside the list's {len(items)} items")
        return items[token]
    table_key = CHILDREN_KEY_OF.get(kind) if kind != "list" else None
    if table_key is None:
        raise ManifestError(f"'{path}': '.{token}' descends into a {kind} node, which has no children")
    table = node[table_key]
    if token not in table:
        raise ManifestError(f"'{path}': no '{token}' here (available: {sorted(table)})")
    return table[token]

# rtl — one emitted design per generated language, as the simulator sees it.
#
#   base.py      Rtl: dir, top module, sources digest, and the hooks a language fills
#   verilog.py   VerilogRtl: *.v sources, port-id normalisation
#
# open_rtl() is the switch: it reads the manifest's "backend" tag and picks the
# class, so a caller never names a language.  A new language is a new file plus
# one row in RTL_BY_BACKEND — the same shape as SIM_NAMERS and BACKENDS.

from .base    import Rtl, read_manifest
from .verilog import VerilogRtl

RTL_BY_BACKEND = {"verilog": VerilogRtl}


def open_rtl(rtl_dir) -> Rtl:
    """The Rtl for an emit dir, chosen by the manifest's backend tag."""
    tag = read_manifest(rtl_dir).get("backend", "verilog")
    if tag not in RTL_BY_BACKEND:
        raise ValueError(f"no Rtl reader for backend '{tag}' — one of {sorted(RTL_BY_BACKEND)}")
    return RTL_BY_BACKEND[tag](rtl_dir)


__all__ = ["Rtl", "VerilogRtl", "RTL_BY_BACKEND", "open_rtl", "read_manifest"]

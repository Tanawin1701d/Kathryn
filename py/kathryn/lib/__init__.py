# kathryn.lib — small standard library of generic, reusable hardware helpers
# built purely on the public DSL (bits, arith, bundles, handshakes). Nothing in
# here knows about any particular application domain.

from .bits      import (width_of, zext, sext, cat, replicate, or_reduce, and_reduce,
                        mux, muxn, decoder, priority_encoder)
from .arith     import mul_wide, mulh
from .bundle    import Bundle
from .handshake import Decoupled

__all__ = [
    "width_of", "zext", "sext", "cat", "replicate", "or_reduce", "and_reduce",
    "mux", "muxn", "decoder", "priority_encoder",
    "mul_wide", "mulh",
    "Bundle", "Decoupled",
]

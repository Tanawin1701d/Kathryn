# manifest — sim_manifest.json: the attribute names a model uses, mapped to the
# names the emitter gave them.  One file, one writer, one reader.
#
#   schema.py   the shape both sides agree on            (stdlib)
#   write.py    BUILD side: walks the live Module tree    (needs the model layer)
#   ../ksim.py  SIM side: the same paths, resolved to live cocotb handles  (stdlib)
#
# NOT re-exported here: write.py.  It is the one module that needs the model
# layer, and `_session` imports it back lazily — keeping it out of this file is
# what lets ksim.py import the schema without dragging in the arena.  Reach it
# as `from kathryn.sim.manifest.write import write_sim_manifest`.

from .schema import CHILDREN_KEY_OF, NODE_KINDS, SCHEMA_VERSION, SIM_MANIFEST_FILE

__all__ = ["CHILDREN_KEY_OF", "NODE_KINDS", "SCHEMA_VERSION", "SIM_MANIFEST_FILE"]

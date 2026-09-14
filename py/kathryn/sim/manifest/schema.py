# Manifest SCHEMA — the ONE definition of sim_manifest.json's shape and name.
# - The writer (write.py) builds nodes from it and the reader (ksim.py, in the
#   simulator) dispatches on it, so a renamed key or a new kind cannot land on
#   one side only.
# - Stdlib-only, and imports nothing from this package: everything else here
#   imports it, so it must not import back.

SCHEMA_VERSION    = 1
SIM_MANIFEST_FILE = "sim_manifest.json"       # written next to the emitted HDL
NODE_KINDS        = ("module", "signal", "slice", "counter", "karray", "list", "dict")
CHILDREN_KEY_OF   = {"module": "children",    # the key a container kind keeps its children under
                     "dict"  : "entries",
                     "list"  : "items"}

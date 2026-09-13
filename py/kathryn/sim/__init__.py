# sim — everything about turning an emitted model into a running simulator, and
# finding your way around what it emitted.
# - Two halves that never meet: the BUILD side answers questions about an emit
#   (what is where, what is it called, how do I compile it); the SIM side runs
#   inside the simulator subprocess and is stdlib-only.
# - Nothing here imports kathryn.observe or kathryn.view.
#
#   ksim.py           SIM side: KSim(dut) — reach internal signals by model name
#   manifest/         sim_manifest.json: schema.py · write.py (BUILD side, needs the model) · read.py (as a tree)
#   verilog_scope.py  BUILD side: which module DECLARES a net, read off the .v
#                     (TODO: DELETE next release — the arena will say instead)
#   rtl/              one emitted design per generated language: sources, top, fingerprint
#   backend/          which simulator: a harness-neutral base, then one sub-package per harness (cocotb/)
#   runner_cocotb.py  CocotbSim: the fingerprint-cached build, and its run_test
#
# NOT re-exported here: manifest/write.py.  It is the one module that needs the
# model layer (`..module`, `.._session`), and `_session` imports it back lazily
# — keeping it out of this file is what stops that becoming a cycle.
# Reach it as `from kathryn.sim.manifest.write import write_sim_manifest`.

from .ksim import (KSim, KSimKarray, KSimKarrayElement, KSimModule, KSimSlice,
                   SIM_MANIFEST_ENV, TOP_CLOCK_PORT, TOP_RESET_PORT)
from .backend import SimBackend
from .backend.cocotb import (BACKENDS, CocotbBackend, IcarusBackend, VerilatorBackend, cocotb_is_available,
                             get_backend, icarus_is_available, verilator_is_available)
from .manifest import (CHILDREN_KEY_OF, ManifestError, Manifest, NODE_KINDS, SCHEMA_VERSION,
                       SIM_MANIFEST_FILE, child_node, parse_path, walk_path)
from .rtl import RTL_BY_BACKEND, Rtl, VerilogRtl, open_rtl
from .runner_cocotb import CocotbSim, read_test_status
from .verilog_scope import Declaration, LocateError, VerilogScope, find_home_module

__all__ = ["KSim"             , "KSimKarray"         , "KSimKarrayElement"     , "KSimModule"          , "KSimSlice",
           "SIM_MANIFEST_ENV" , "SIM_MANIFEST_FILE"  , "TOP_CLOCK_PORT"        , "TOP_RESET_PORT"      ,
           "SCHEMA_VERSION"   , "NODE_KINDS"         , "CHILDREN_KEY_OF"       ,
           "BACKENDS"         , "IcarusBackend"      , "SimBackend"            , "VerilatorBackend"    , "cocotb_is_available",
           "get_backend"      , "icarus_is_available", "verilator_is_available", "CocotbBackend"       ,
           "ManifestError"    , "Manifest"       , "child_node"            , "parse_path"          , "walk_path",
           "CocotbSim"        , "read_test_status"   ,
           "Rtl"              , "VerilogRtl"         , "RTL_BY_BACKEND"        , "open_rtl"            ,
           "Declaration"      , "LocateError"        , "VerilogScope"          , "find_home_module"]

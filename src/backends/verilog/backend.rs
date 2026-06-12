use crate::backends::common::graph::DfsModuleIter;
use crate::backends::common::glob_routing::route_glob_io_model;
use crate::backends::common::internal_routing::route_and_remap_io_model;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;


pub struct BackendVerilog {
    model_arena : ModelArena,
}

impl BackendVerilog {

    // Takes ownership of the arena — the backend is the final consumer.
    pub fn new(model_arena: ModelArena) -> Self {
        Self { model_arena }
    }

    // Top-level entry point: run all three phases in order.
    // output_dir must already exist; each module produces <output_dir>/<name>.v,
    // except the top module which is written to <output_dir>/<top_file_name>.v.
    pub fn emit(&mut self, output_dir: &str, top_file_name: &str) {
        self.phase_route();
        self.phase_init();
        self.phase_emit(output_dir, top_file_name);
    }

    // ---- Phases ----

    // Phase 1 — route cross-module signal dependencies and insert IoWire chains.
    fn phase_route(&mut self) {
        println!("[BackendVerilog] Phase 1: routing cross-module IO dependencies");
        route_and_remap_io_model(&mut self.model_arena);
        route_glob_io_model(&mut self.model_arena);
        println!("[BackendVerilog] Phase 1: done");
    }

    // Phase 2 — sort every HCP's UpdatePool across the whole module tree.
    fn phase_init(&mut self) {
        println!("[BackendVerilog] Phase 2: initialising HCP update pools");
        let top_i = self.model_arena
            .get_top_module()
            .expect("BackendVerilog::phase_init — no top module set on arena");
        let top_module = self.model_arena.take_module(top_i);
        top_module.init_module_vb(&mut self.model_arena);
        self.model_arena.replace_back_module(top_i, top_module);
        println!("[BackendVerilog] Phase 2: done");
    }

    // Phase 3 — DFS over the module tree; emit one .v file per module.
    // The top module is written to <top_file_name>.v; all others use their module name.
    fn phase_emit(&mut self, output_dir: &str, top_file_name: &str) {
        println!("[BackendVerilog] Phase 3: emitting Verilog to '{output_dir}'");
        let top_i = self.model_arena
            .get_top_module()
            .expect("BackendVerilog::phase_emit — no top module set on arena");
        let mut iter = DfsModuleIter::new(top_i);
        while let Some(module_i) = iter.next_module(&mut self.model_arena) {
            let module    = self.model_arena.take_module(module_i);
            let file_stem = if module_i == top_i { top_file_name.to_string() }
                            else                 { module.get_mod_name_vb() };
            let path      = format!("{}/{}.v", output_dir, file_stem);
            println!("[BackendVerilog] Phase 3:   writing {path}");
            let mut fw = FileWriter::new(&path, 4096)
                .unwrap_or_else(|e| panic!("BackendVerilog::phase_emit — cannot open {path}: {e}"));
            module.gen_module_blk_vb(&mut self.model_arena, &mut fw);
            self.model_arena.replace_back_module(module_i, module);
        }
        println!("[BackendVerilog] Phase 3: done");
    }

}

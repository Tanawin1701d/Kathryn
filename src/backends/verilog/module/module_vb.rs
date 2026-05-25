use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::model_arena::ModelArena;
use crate::model::module::module::Module;
use crate::util::file::file_writer::FileWriter;

// Produce every HwComponentType variant in discriminant order.
// Safe because the enum is #[repr(usize)] with contiguous variants 0..COUNT.
fn all_hw_types() -> impl Iterator<Item = HwComponentType> {
    (0..HwComponentType::COUNT)
        .map(|i| unsafe { std::mem::transmute::<usize, HwComponentType>(i) })
}

impl Module {

    // ---- init phase ----

    // Sort every HCP's UpdatePool in this module and recurse into sub-modules.
    // `self` must already be taken from the arena so arena is free for sub-module access.
    pub fn init_hw_component(&self, arena: &mut ModelArena) {
        self.sort_update_event_pool(arena);

        let sub_module_ids = self.get_user_sub_modules().clone();
        for sub_module_i in sub_module_ids {
            let sub_module = arena.take_module(sub_module_i);
            sub_module.init_hw_component(arena);
            arena.replace_back_module(sub_module_i, sub_module);
        }
    }

    // ---- Verilog module file generation — master ----

    // Master emitter: glues all five phases into a complete `module … endmodule` block.
    // `self` must already be taken from the arena so arena is free for HCP access.
    pub fn gen_module_blk(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        self.gen_module_header   (arena, fw);
        self.gen_var_declarations(arena, fw);
        self.gen_procedure_blks  (arena, fw);
        self.gen_sub_module_insts(arena, fw);
        self.gen_module_footer   (       fw);
    }

    // ---- Verilog module file generation — per-phase helpers ----

    // Phase 1 — `module name( <io ports>, input wire clk );`
    fn gen_module_header(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        let mod_name = self.get_global_name().to_string();
        fw.write("////////////////////////////////////////////////////////////////////////////////\n");
        fw.write(&format!("module {mod_name}(\n"));

        for &io_i in &self.collect_hcp_idents(HwComponentType::IoWire) {
            let vb = arena.take_hcp_vb(io_i);
            for idx in 0..vb.amt_io_line_vb() {
                fw.write("    ");
                vb.gen_io_line_vb(idx, arena, fw);
                fw.write(",\n");
            }
            arena.replace_back_hcp_vb(vb);
        }
        fw.write("    input wire clk\n);\n\n");
    }

    // Phase 2 — signal declarations: reg, wire, localparam, mem.
    // IoWire is skipped — its signals are already declared as ports in phase 1.
    fn gen_var_declarations(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        for hw_type in all_hw_types() {
            if hw_type == HwComponentType::IoWire { continue; }
            for &hcp_i in &self.collect_hcp_idents(hw_type) {
                let vb = arena.take_hcp_vb(hcp_i);
                for idx in 0..vb.amt_init_line_vb() {
                    vb.gen_init_line_vb(idx, arena, fw);
                    fw.write("\n");
                }
                arena.replace_back_hcp_vb(vb);
            }
        }
        fw.write("\n");
    }

    // Phase 3 — always blocks and assign statements for every HCP.
    fn gen_procedure_blks(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        for hw_type in all_hw_types() {
            for &hcp_i in &self.collect_hcp_idents(hw_type) {
                let vb = arena.take_hcp_vb(hcp_i);
                for idx in 0..vb.amt_precedure_blk_vb() {
                    vb.gen_procedure_blk_vb(idx, arena, fw);
                    fw.write("\n");
                }
                arena.replace_back_hcp_vb(vb);
            }
        }
    }

    // Phase 4 — instantiate every child module inside this module's body.
    fn gen_sub_module_insts(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        let sub_ids = self.get_user_sub_modules().clone();
        for sub_i in sub_ids {
            let sub_module = arena.take_module(sub_i);
            sub_module.gen_as_sub_module_dec(arena, fw);
            arena.replace_back_module(sub_i, sub_module);
        }
    }

    // Phase 5 — `endmodule`.
    fn gen_module_footer(&self, fw: &mut FileWriter) {
        fw.write("\nendmodule\n\n");
    }

    // ---- Sub-module instantiation ----

    // Emit the named-port instantiation block for this module inside its parent.
    // Input ports wire to their agent_src (already in parent scope via routing);
    // output ports drive an implicit wire in the parent whose name matches the port.
    pub fn gen_as_sub_module_dec(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        let mod_name = self.get_global_name().to_string();
        fw.write(&format!("{mod_name}  {mod_name} (\n"));

        for &io_i in &self.collect_hcp_idents(HwComponentType::IoWire) {
            let io_wire   = arena.take_io_wire(io_i);
            let port_name = io_wire.get_global_name().to_string();
            let connected = if io_wire.get_is_input() {
                io_wire.get_agent_src_signal_i().get_global_name().to_string()
            } else {
                port_name.clone()   // output: port name IS the implicit wire in parent scope
            };
            fw.write(&format!("    .{port_name}({connected}),\n"));
            arena.replace_back_io_wire(io_wire);
        }
        fw.write("    .clk(clk)\n);\n");
    }

    // ---- Private utilities ----

    // Collect all HCP idents of `hw_type` from both internal and user pools.
    fn collect_hcp_idents(&self, hw_type: HwComponentType) -> Vec<HcpIdent> {
        self.get_internal_hws(hw_type)
            .iter()
            .chain(self.get_user_hws(hw_type).iter())
            .copied()
            .collect()
    }
}

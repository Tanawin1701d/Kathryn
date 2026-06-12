use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType, all_hw_types};
use crate::model::model_arena::ModelArena;
use crate::model::module::module::Module;
use crate::util::file::file_writer::FileWriter;

impl Module {

    // ---- init phase ----

    // Sort every HCP's UpdatePool in this module and recurse into sub-modules.
    // `self` must already be taken from the arena so arena is free for sub-module access.
    pub fn init_module_vb(&self, arena: &mut ModelArena) {
        self.sort_update_event_pool(arena);

        let sub_module_ids = self.get_user_sub_modules().clone();
        for sub_module_i in sub_module_ids {
            let sub_module = arena.take_module(sub_module_i);
            sub_module.init_module_vb(arena);
            arena.replace_back_module(sub_module_i, sub_module);
        }
    }

    // ---- Verilog module file generation — master ----

    // Master emitter: glues all phases into a complete `module … endmodule` block.
    // `self` must already be taken from the arena so arena is free for HCP access.
    pub fn gen_module_blk_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        self.gen_module_header_vb(arena, fw);
        self.gen_var_declarations_vb(arena, fw);
        self.gen_var_sub_mod_declarations_vb(arena, fw);
        self.gen_procedure_blks_vb(arena, fw);
        self.gen_sub_module_insts_vb(arena, fw);
        self.gen_module_footer_vb(       fw);
    }

    // ---- Verilog module file generation — per-phase helpers ----

    // Phase 1 — `module name( <io ports>, input wire clk );`
    fn gen_module_header_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        let mod_name = self.get_mod_name_vb();
        fw.write("////////////////////////////////////////////////////////////////////////////////\n");
        fw.write("// Phase 1 : module header & IO ports\n");
        fw.write(&format!("module {mod_name}(\n"));

        // Each IoWire is taken from the arena so `arena` stays free for recursive access
        // inside gen_io_line_vb, then put back immediately after.
        // amt_io_line_vb() tells how many port declaration lines this HCP contributes
        // (usually 1, but a future multi-port IoWire could emit more).
        // Verilog forbids a trailing comma after the last port, so commas are written
        // *before* each port except the first (need_comma gate); the final port's line
        // is closed with a bare "\n" after the loop.
        // Emits:  "    portA,\n    portB,\n    portC\n);\n"
        let mut need_comma = false;
        for &io_i in &self.collect_hcp_idents_vb(HwComponentType::IoWire) {
            let vb = arena.take_hcp_vb(io_i);  // take → arena is free
            for idx in 0..vb.amt_io_line_vb() {
                if need_comma { fw.write(",\n"); }            // finish previous line
                fw.write("    ");
                vb.gen_io_line_vb(idx, arena, fw);                      // write port (no newline yet)
                need_comma = true;
            }
            arena.replace_back_hcp_vb(vb);                       // return to arena
        }
        if need_comma { fw.write("\n"); }                     // close last port (no comma)
        fw.write(");\n\n");
    }

    // Phase 2 — signal declarations: reg, wire, localparam, mem.
    // IoWire is skipped for *this* module — its signals are already declared as ports in phase 1.
    // Sub-module output IoWires are declared here as plain wires (child drives; parent just names them).
    fn gen_var_declarations_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        self.gen_phase_banner_vb(fw, "Phase 2 : signal declarations (reg / wire / localparam / mem)");
        for hw_type in all_hw_types() {
            if hw_type == HwComponentType::IoWire { continue; }
            let hcp_idents = self.collect_hcp_idents_vb(hw_type);
            if hcp_idents.is_empty() { continue; }   // no banner for types this module never uses
            self.gen_phase_banner_vb(fw, &format!("{} declarations", hw_type.global_prefix()));
            for &hcp_i in &hcp_idents {
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

    // Phase 2.5 — wire declarations for every sub-module's output ports.
    // Each sub-module is taken from the arena so arena is free inside gen_var_sub_mod_declaration.
    fn gen_var_sub_mod_declarations_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        self.gen_phase_banner_vb(fw, "Phase 2.5 : sub-module output wire declarations");
        let sub_ids = self.get_user_sub_modules().clone();
        for sub_i in sub_ids {
            let sub_module = arena.take_module(sub_i);
            sub_module.gen_var_sub_mod_declaration_vb(arena, fw);
            arena.replace_back_module(sub_i, sub_module);
        }
    }

    // Phase 3 — always blocks and assign statements for every HCP.
    fn gen_procedure_blks_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        self.gen_phase_banner_vb(fw, "Phase 3 : always blocks & continuous assignments");
        for hw_type in all_hw_types() {
            for &hcp_i in &self.collect_hcp_idents_vb(hw_type) {
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
    fn gen_sub_module_insts_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        self.gen_phase_banner_vb(fw, "Phase 4 : sub-module instantiations");
        let sub_ids = self.get_user_sub_modules().clone();
        for sub_i in sub_ids {
            let sub_module = arena.take_module(sub_i);
            sub_module.gen_inst_sub_module_declaration_vb(arena, fw);
            arena.replace_back_module(sub_i, sub_module);
        }
    }

    // Phase 5 — `endmodule`.
    fn gen_module_footer_vb(&self, fw: &mut FileWriter) {
        self.gen_phase_banner_vb(fw, "Phase 5 : endmodule");
        fw.write("\nendmodule\n\n");
    }

    // ---- Private utilities ----

    // Emit a Verilog section banner comment so each generation phase is
    // identifiable in the produced file.
    fn gen_phase_banner_vb(&self, fw: &mut FileWriter, text: &str) {
        fw.write(&format!("    // ---- {text} ----\n"));
    }

    // Canonical module name used in Verilog output.
    pub(crate) fn get_mod_name_vb(&self) -> String { self.get_global_name().to_string() }

    // Collect all HCP idents of `hw_type` from both internal and user pools.
    fn collect_hcp_idents_vb(&self, hw_type: HwComponentType) -> Vec<HcpIdent> {
        self.get_internal_hws(hw_type)
            .iter()
            .chain(self.get_user_hws(hw_type).iter())
            .copied()
            .collect()
    }

    // ---- sub module generation for master

    // Emit the named-port instantiation block for this module inside its parent.
    // Input ports wire to their agent_src (already in parent scope via routing);
    // output ports drive an implicit wire in the parent whose name matches the port.
    pub fn gen_inst_sub_module_declaration_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        let mod_name = self.get_mod_name_vb();
        fw.write(&format!("{mod_name}  {mod_name} (\n"));

        let mut need_comma = false;
        for &io_i in &self.collect_hcp_idents_vb(HwComponentType::IoWire) {
            let io_wire   = arena.take_io_wire(io_i);
            let port_name = io_wire.gen_var_name_vb();
            let connected = if io_wire.get_is_input() {
                io_wire.gen_agent_input_vb(arena)       // resolves the agent driver name
            } else {
                io_wire.gen_var_name_vb()               // output: port name IS the wire in parent scope
            };
            if need_comma { fw.write(",\n"); }
            fw.write(&format!("    .{port_name}({connected})"));
            arena.replace_back_io_wire(io_wire);
            need_comma = true;
        }
        if need_comma { fw.write("\n"); }
        fw.write(");\n");
    }

    // Declare this module's output ports as wires in the parent scope.
    // Output ports are driven by this child; the parent only needs the net name for
    // connectivity.  Input ports are omitted — they are driven by parent signals
    // already declared in the parent's own var-declaration block.
    fn gen_var_sub_mod_declaration_vb(&self, arena: &mut ModelArena, fw: &mut FileWriter) {
        // The routing pass already registered each sub-module output as an IoWire
        // on *this* (the master) module — no need to dive into sub-modules.
        let io_idents = self.collect_hcp_idents_vb(HwComponentType::IoWire);
        for io_i in io_idents {
            let io_wire = arena.take_io_wire(io_i);
            if !io_wire.get_is_input() {
                // Output port of a child → declare as a wire in this parent scope.
                io_wire.gen_init_line_vb(0, arena, fw);
                fw.write("\n");
            }
            arena.replace_back_io_wire(io_wire);
        }
    }

}

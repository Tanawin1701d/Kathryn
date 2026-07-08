use crate::backends::verilog::hw_component::util_vb::{fmt_operand, slice_to_verilog};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UeGrp, UeSwitch};
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

// ---- Trait ----

pub trait VerilogUpdateEvent {

    // op_template contains the destination lvalue with placeholders:
    //   `des_val{DES_SLICE} <= {SRC};`
    // transpile fills {DES_SLICE} from the UE's des_slice and {SRC} from fmt_operand,
    // then writes the resulting lines directly into fw.
    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        arena       : &mut ModelArena,
        active_i    : HcpIdent, // HCP currently taken from arena by the caller
        active_name : &str,     // gen_var_name() of active_i (can't re-take it)
        active_size : i32,      // bit width of active_i (can't re-take it either)
        fw          : &mut FileWriter,
    );

    // Each concrete type puts itself back into the correct typed arena slot.
    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena);
}

// ---- Dispatch helper — ZERO match (match lives in arena_ext_vb::take_ue_vb) ----

/// Transpile any update event by handle, writing directly into fw.
/// New UE types only require a new arm in `ModelArena::take_ue_vb`.
pub fn transpile_ue(
    ue_i        : UpdateEventIdent,
    op_templates: Vec<String>,
    front_space : u32,
    arena       : &mut ModelArena,
    active_i    : HcpIdent,
    active_name : &str,
    active_size : i32,
    fw          : &mut FileWriter,
) {
    let ue = arena.take_ue_vb(ue_i);
    ue.transpile(op_templates, front_space, arena, active_i, active_name, active_size, fw);
    ue.replace_back_into_arena_vb(arena);
}

// ---- impl VerilogUpdateEvent for UeBasic ----

impl VerilogUpdateEvent for UeBasic {

    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        arena       : &mut ModelArena,
        active_i    : HcpIdent,
        active_name : &str,
        active_size : i32,
        fw          : &mut FileWriter,
    ) {
        let sp        = " ".repeat(front_space as usize);
        // The destination is the active HCP; a 1-bit destination is declared
        // scalar, so its slice must be elided (part-select on scalar is illegal).
        let des_slice = if active_size <= 1 { String::new() }
                        else { slice_to_verilog(&self.get_des_slice()) };
        // Resolve the source name via gen_var_name() — respects per-type name overrides.
        let src_str   = fmt_operand(
            self.get_srci_val(), Some(self.get_src_slice()), arena, active_i, active_name, active_size,
        );

        for tmpl in &op_templates {
            let line = tmpl.replace("{DES_SLICE}", &des_slice)
                           .replace("{SRC}",       &src_str);
            fw.write(&format!("{sp}{line}\n"));
        }
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_basic(*self);
    }
}

// ---- impl VerilogUpdateEvent for UeGrp ----

impl VerilogUpdateEvent for UeGrp {

    // Sequential join — no enclosing block, just write each sub-stmt in order.
    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        arena       : &mut ModelArena,
        active_i    : HcpIdent,
        active_name : &str,
        active_size : i32,
        fw          : &mut FileWriter,
    ) {
        for &sub_i in self.get_sub_stmts() {
            transpile_ue(sub_i, op_templates.clone(), front_space, arena, active_i, active_name, active_size, fw);
        }
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_grp(*self);
    }
}

// ---- impl VerilogUpdateEvent for UeCond ----

impl VerilogUpdateEvent for UeCond {

    // if / else-if / else chain.  `end` for non-terminal branches has no trailing
    // newline so the next iteration can append " else …" on the same line.
    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        arena       : &mut ModelArena,
        active_i    : HcpIdent,
        active_name : &str,
        active_size : i32,
        fw          : &mut FileWriter,
    ) {
        let sp = " ".repeat(front_space as usize);
        let n  = self.get_conditions().len();

        for i in 0..n {
            let cond_opt = &self.get_conditions()[i];
            let stmt_opt = &self.get_sub_stmts()[i];

            // ---- open branch ----
            if i == 0 {
                match cond_opt {
                    Some(c) => {
                        let c_str = fmt_operand(*c, None, arena, active_i, active_name, active_size);
                        fw.write(&format!("{sp}if ({c_str}) begin\n"));
                    }
                    None    => fw.write(&format!("{sp}begin\n")),
                }
            } else {
                match cond_opt {
                    Some(c) => {
                        let c_str = fmt_operand(*c, None, arena, active_i, active_name, active_size);
                        fw.write(&format!(" else if ({c_str}) begin\n"));
                    }
                    None    => fw.write(" else begin\n"),
                }
            }

            // ---- body ----
            if let Some(sub_i) = stmt_opt {
                transpile_ue(*sub_i, op_templates.clone(), front_space + 4, arena, active_i, active_name, active_size, fw);
            }

            // ---- close branch ----
            if i < n - 1 {
                fw.write(&format!("{sp}end")); // no trailing newline — next iter appends " else"
            } else {
                fw.write(&format!("{sp}end\n"));
            }
        }
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_cond(*self);
    }
}

// ---- impl VerilogUpdateEvent for UeSwitch ----

impl VerilogUpdateEvent for UeSwitch {

    // Verilog `case` block; case values are decimal integers.
    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        arena       : &mut ModelArena,
        active_i    : HcpIdent,
        active_name : &str,
        active_size : i32,
        fw          : &mut FileWriter,
    ) {
        let sp      = " ".repeat(front_space as usize);
        let case_sp = " ".repeat((front_space + 4) as usize);
        let state   = fmt_operand(*self.get_state_iden(), None, arena, active_i, active_name, active_size);
        fw.write(&format!("{sp}case ({state})\n"));

        for i in 0..self.get_match_num() {
            let val = self.get_sub_stmt_match_idx(i);
            fw.write(&format!("{case_sp}{val}: begin\n"));
            if let Some(sub_i) = self.get_sub_stmt(i) {
                transpile_ue(sub_i, op_templates.clone(), front_space + 8, arena, active_i, active_name, active_size, fw);
            }
            fw.write(&format!("{case_sp}end\n"));
        }

        fw.write(&format!("{sp}endcase\n"));
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_switch(*self);
    }
}

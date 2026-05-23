use crate::backends::verilog::hw_component::util_vb::slice_to_verilog;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UeGrp, UeSwitch};
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

// ---- Trait ----

pub trait VerilogUpdateEvent {

    // op_template is the template string used to placed as main operation in update event
    // it may have many pattern such as des_val{DES_SLICE} <= {SRC_VAL}{SRC_SLICE};
    // the transpile will build the precedure block and fill DES_SLICE pattern
    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        arena       : &mut ModelArena,
    ) -> String;

    // Each concrete type puts itself back into the correct typed arena slot.
    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena);
}

// ---- Dispatch helper — ZERO match (match lives in arena_ext_vb::take_ue_vb) ----

/// Transpile any update event by handle, without a match at the call site.
/// New UE types only require a new arm in `ModelArena::take_ue_vb`.
pub fn transpile_ue(
    ue_i        : UpdateEventIdent,
    op_templates: Vec<String>,
    front_space : u32,
    arena       : &mut ModelArena,
) -> String {
    let ue = arena.take_ue_vb(ue_i);
    let s  = ue.transpile(op_templates, front_space, arena);
    ue.replace_back_into_arena_vb(arena);
    s
}

// ---- impl VerilogUpdateEvent for UeBasic ----

impl VerilogUpdateEvent for UeBasic {

    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        _arena      : &mut ModelArena,
    ) -> String {
        let sp        = " ".repeat(front_space as usize);
        let des_slice = slice_to_verilog(self.get_des_slice());
        let src_slice = slice_to_verilog(self.get_src_slice());
        let src_name  = self.get_srci_val().get_global_name();

        let mut out = String::new();
        for tmpl in &op_templates {
            let line = tmpl.replace("{DES_SLICE}", &des_slice)
                           .replace("{SRC_VAL}",   src_name)
                           .replace("{SRC_SLICE}", &src_slice);
            out += &format!("{}{}\n", sp, line);
        }
        out
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_basic(*self);
    }
}

// ---- impl VerilogUpdateEvent for UeGrp ----

impl VerilogUpdateEvent for UeGrp {

    // Sequential join — no enclosing block, just concatenate each sub-stmt.
    fn transpile(
        &self,
        op_templates: Vec<String>,
        front_space : u32,
        arena       : &mut ModelArena,
    ) -> String {
        self.get_sub_stmts().iter().map(|&sub_i| {
            transpile_ue(sub_i, op_templates.clone(), front_space, arena)
        }).collect()
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
    ) -> String {
        let sp      = " ".repeat(front_space as usize);
        let n       = self.get_conditions().len();
        let mut out = String::new();

        for i in 0..n {
            let cond_opt = &self.get_conditions()[i];
            let stmt_opt = &self.get_sub_stmts()[i];

            // ---- open branch ----
            if i == 0 {
                match cond_opt {
                    Some(c) => out += &format!("{}if ({}) begin\n", sp, c.get_global_name()),
                    None    => out += &format!("{}begin\n", sp),
                }
            } else {
                match cond_opt {
                    Some(c) => out += &format!(" else if ({}) begin\n", c.get_global_name()),
                    None    => out += " else begin\n",
                }
            }

            // ---- body ----
            if let Some(sub_i) = stmt_opt {
                out += &transpile_ue(*sub_i, op_templates.clone(), front_space + 4, arena);
            }

            // ---- close branch ----
            if i < n - 1 {
                out += &format!("{}end", sp); // no newline — next iter prepends " else"
            } else {
                out += &format!("{}end\n", sp);
            }
        }

        out
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
    ) -> String {
        let sp      = " ".repeat(front_space as usize);
        let case_sp = " ".repeat((front_space + 4) as usize);
        let state   = self.get_state_iden().get_global_name();
        let mut out = format!("{}case ({})\n", sp, state);

        for i in 0..self.get_match_num() {
            let val = self.get_sub_stmt_match_idx(i);
            out += &format!("{}{}: begin\n", case_sp, val);
            if let Some(sub_i) = self.get_sub_stmt(i) {
                out += &transpile_ue(sub_i, op_templates.clone(), front_space + 8, arena);
            }
            out += &format!("{}end\n", case_sp);
        }

        out += &format!("{}endcase\n", sp);
        out
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_switch(*self);
    }
}

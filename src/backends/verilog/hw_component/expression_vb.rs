use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::{fmt_operand, logic_op_to_verilog, signal_width};
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::expression::Expression;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

impl HcpBaseVb for Expression {
    fn gen_type_vb         (&self) -> String { format!("wire {}", signal_width(self.get_des_slice().get_size())) }
    fn gen_var_name_vb     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line_vb    (&self) -> u32    { 1 }
    fn amt_precedure_blk_vb(&self) -> u32    { 1 }

    fn gen_init_line_vb(&self, _idx: u32, _arena: &mut ModelArena, fw: &mut FileWriter) {
        let ty   = self.gen_type_vb();
        let name = self.gen_var_name_vb();
        fw.write(&format!("{ty} {name};"));
    }

    fn gen_procedure_blk_vb(&self, _idx: u32, arena: &mut ModelArena, fw: &mut FileWriter) {
        if !self.is_value_assigned() { return; }

        // Hoist once — all fmt_operand calls in this block share the same active context.
        let active_i    = self.get_ident();
        let active_name = self.gen_var_name_vb();
        let active_size = self.get_des_slice().get_size();

        let a_i   = match self.get_operand_a() { Some(x) => x, None => return };
        let a_str = fmt_operand(a_i, self.get_operand_a_slice(), arena, active_i, &active_name, active_size);

        let rhs = match self.get_op() {

            // ---- wire-through: no operator token ----
            // SliceBit emits `a[slice]`; fmt_operand already appended the slice to a_str.
            LogicOp::Assign | LogicOp::SliceBit => a_str,

            // ---- prefix unary: ~a, !a ----
            op if op.is_single_opr() => {
                let op_tok = logic_op_to_verilog(op);
                format!("{op_tok}{a_str}")
            }

            // ---- arithmetic shift right: $signed(a) >>> b ----
            // Only the left operand is cast: a shift's result signedness follows
            // the left operand alone (IEEE 1800), and the amount stays unsigned.
            LogicOp::ArithShrA => {
                let b_i   = match self.get_operand_b() { Some(x) => x, None => return };
                let b_str = fmt_operand(b_i, self.get_operand_b_slice(), arena, active_i, &active_name, active_size);
                format!("$signed({a_str}) >>> {b_str}")
            }

            // ---- signed relational / arithmetic: $signed(a) OP $signed(b) ----
            LogicOp::RelationSlt | LogicOp::RelationSgt | LogicOp::ArithDivS | LogicOp::ArithRemS => {
                let b_i    = match self.get_operand_b() { Some(x) => x, None => return };
                let b_str  = fmt_operand(b_i, self.get_operand_b_slice(), arena, active_i, &active_name, active_size);
                let op_tok = logic_op_to_verilog(self.get_op());
                format!("$signed({a_str}) {op_tok} $signed({b_str})")
            }

            // ---- bit extension: {{ext_count{fill}}, a[slice]} ----
            // operand_b is the fill bit (e.g. sign bit); defaults to 1'b0 for zero extension.
            // ext_count = result width − source slice width.
            LogicOp::ExtendBit => {
                let bit_width    = self.get_des_slice().get_size();
                let a_slice_size = self.get_operand_a_slice()
                                       .map_or(bit_width, |s| s.get_size());
                let ext_count    = bit_width - a_slice_size;

                if ext_count <= 0 {
                    // source already covers the full width — no padding needed
                    a_str
                } else {
                    let fill = match self.get_operand_b() {
                        Some(b_i) => fmt_operand(b_i, self.get_operand_b_slice(), arena, active_i, &active_name, active_size),
                        None      => "1'b0".to_string(),
                    };
                    // Verilog: {{ext_count{fill}}, a_str}
                    // Two-step to sidestep Rust's brace-escape rules.
                    let repl = format!("{ext_count}{{{fill}}}");  // e.g. "3{1'b0}"
                    format!("{{{{{repl}}}, {a_str}}}")            // e.g. "{{3{1'b0}}, sig[3:0]}"
                }
            }

            // ---- generic binary: a OP b ----
            op => {
                let b_i    = match self.get_operand_b() { Some(x) => x, None => return };
                let b_str  = fmt_operand(b_i, self.get_operand_b_slice(), arena, active_i, &active_name, active_size);
                let op_tok = logic_op_to_verilog(op);
                format!("{a_str} {op_tok} {b_str}")
            }
        };

        let name = self.gen_var_name_vb();
        fw.write(&format!("assign {name} = {rhs};\n"));
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_expression(*self); }
}

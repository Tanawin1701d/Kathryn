use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::model_arena::ModelArena;

// ---- Karray static-index resolution -----------------------------------------

impl Karray {
    /// Resolve a static index + field selection to that field's own HCP, the
    /// target of `|=` / `*=`. Reg/Wire return the per-(element,field) HCP directly;
    /// MemBlock builds a constant-address read/write MemEle on the field's block.
    pub fn static_index_get_hcp(&self, indices: &[usize], field_idx: usize, is_read: bool, arena: &mut ModelArena) -> HcpIdent {
        let nf = self.field_count();
        assert!(field_idx < nf, "Karray: field index {field_idx} out of range (have {nf})");
        let flat = self.flat_index(indices);
        match self.get_backing() {
            HwComponentType::MemBlock => {
                let blk_i  = self.get_backing_hcps()[field_idx];
                let fwidth = self.get_fields()[field_idx].get_width();
                let iw     = self.index_width();
                let base   = self.get_ccp_ident().get_global_name().to_string();
                let addr_i = arena.make_val(false, &format!("{base}_ADDR{flat}_F{field_idx}"), iw, flat as u64);
                arena.make_mem_ele(false, &format!("{base}_OP{flat}_F{field_idx}"), blk_i, addr_i, fwidth, is_read)
            }
            // Reg / Wire: the per-(element,field) HCP is the destination directly.
            _ => self.get_backing_hcps()[flat * nf + field_idx],
        }
    }
}

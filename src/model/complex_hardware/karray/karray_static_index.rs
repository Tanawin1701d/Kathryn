use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::hw_component::common::assign_meta::AssignMeta;
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

    /// Multi-source element assignment: each `(field_name, src_i)` drives the field
    /// of that name at `indices` from its own source HCP — a full-width connect, with
    /// no bit-level split of a packed source. All field writes are joined into ONE
    /// basic node (so a seq block advances a single cycle, not one cycle per field)
    /// and attached to the current scope. Returns the source names that matched no
    /// field (caller may warn).
    pub fn assign_element(&self, indices: &[usize], sources: &[(String, HcpIdent)], arena: &mut ModelArena) -> Vec<String> {
        let mut metas: Vec<AssignMeta> = Vec::with_capacity(sources.len());
        let mut skipped                = Vec::new();
        for (name, src_i) in sources {
            match self.field_index(name) {
                Some(field_idx) => {
                    let des_i    = self.static_index_get_hcp(indices, field_idx, false, arena);
                    // full-width source, auto-resized to the field; record ONE AssignMeta
                    // (joined into a single node below — not a node per field).
                    let src_full      = arena.get_hw_slice(src_i);
                    let (am, _resize) = arena.gen_asm_meta(des_i, *src_i, None, src_full);
                    metas.push(am);
                }
                None => skipped.push(name.clone()),
            }
        }

        // ---- join all field writes into a single basic node, attach to current scope ----
        if !metas.is_empty() {
            let name   = format!("{}_elem_asm", self.get_ccp_ident().get_global_name());
            let node_i = arena.make_asm_node_many(&name, &metas);
            arena.attach_basic_node_to_current_scope(node_i);
        }
        skipped
    }
}

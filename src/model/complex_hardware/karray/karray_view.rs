use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::kidx::KarrayErr;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// ===== KView — the result of a Karray READ ====================================
//
// The ONE value flowing between the engines: `karray_read.rs` produces it,
// `karray_write.rs` consumes it.
// - A k2k copy is just that composition (done by the arena/connector proxies);
//   NOT here: no third engine exists.
// - Engine-internal — a KView never crosses into the DSL.
// - Every selection collapses to one element, so a view is that element's
//   fields: one HCP per field (the backing HCP when static, else the
//   mux/reduce tree root). Scalar / int / map sources wrap into the same form.

// How the write engine pairs destination fields against this view's fields.
// Stamped by the constructor (constructor-declares-its-own-trait pattern):
// the two source kinds have opposite pairing semantics, and a central match
// re-deriving them from context would be a maintenance trap.
pub enum KViewPairing {
    Exact,   // built by read_view (k2k): pair by exact name + width, unmatched dst -> skipped
    Named,   // built from scalar sources: names already canonical, pair by name, width free
}

pub struct KView {
    fields : Vec<(String, i32)>,  // declared (name, width) per view field, view order
    hcps   : Vec<HcpIdent>,       // one per field, parallel to `fields`
    pairing: KViewPairing,
}

impl KView {
    // ---- constructors ------------------------------------------------------

    /// Read result (k2k source side): the resolved element's fields.
    pub(crate) fn from_read(fields: Vec<(String, i32)>, hcps: Vec<HcpIdent>) -> Self {
        debug_assert_eq!(fields.len(), hcps.len(), "KView: one HCP per field");
        Self { fields, hcps, pairing: KViewPairing::Exact }
    }

    /// View over already-resolved `(field_idx, src_i)` pairs (the scalar / map
    /// assignment path). Names and declared widths are read off the DESTINATION
    /// karray, so pairing later is by-name against canonical names.
    pub(crate) fn from_sources(
        karray : &Karray,
        sources: &[(usize, HcpIdent)],
    ) -> Result<Self, KarrayErr> {
        let nf = karray.field_count();
        if let Some(&(bad, _)) = sources.iter().find(|(field_idx, _)| *field_idx >= nf) {
            return Err(KarrayErr::Value(format!(
                "source field index {bad} out of range (have {nf} fields)")));
        }
        let mut fields = Vec::with_capacity(sources.len());
        let mut hcps   = Vec::with_capacity(sources.len());
        for &(field_idx, src_i) in sources {
            let field = &karray.get_fields()[field_idx];
            fields.push((field.get_name().to_string(), field.get_width()));
            hcps.push(src_i);
        }
        Ok(Self { fields, hcps, pairing: KViewPairing::Named })
    }

    // ---- accessors ---------------------------------------------------------
    pub(crate) fn get_fields (&self) -> &[(String, i32)]           { &self.fields              }
    pub(crate) fn get_pairing(&self) -> &KViewPairing              { &self.pairing             }
    pub(crate) fn hcp        (&self, view_field_idx: usize) -> HcpIdent { self.hcps[view_field_idx] }
}

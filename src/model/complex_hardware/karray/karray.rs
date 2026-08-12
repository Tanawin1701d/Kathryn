use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::complex_hardware::common::ccp_base::CcpBase;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::complex_hardware::karray::karray_meta::{KarrayField, KarrayType};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::model_arena::ModelArena;

// Which kinds of hardware may back a Karray's elements: Reg (clocked, `|=`) or
// Wire (combinational, `*=`). Each materialises one HCP per (element, field).
pub const KARRAY_BACKINGS: [HwComponentType; 2] =
    [HwComponentType::Reg, HwComponentType::Wire];

// ---- Karray -----------------------------------------------------------------

/// A typed multi-dimensional array — a complex component property (CCP). Each
/// field of each element is its **own** HCP (not a bit-slice of a packed
/// element): `total * field_count` per-field HCPs, row-major element then field.
/// Every access selects each dimension with a `KIdx` (see `kidx.rs`); the
/// read/write engines resolve that selection to these backing HCPs. Backing
/// hardware is fixed at construction; there is no internal graph to wire, so
/// `build` is a no-op.
pub struct Karray {
    ident        : CcpIdent,
    shape        : Vec<usize>,        // e.g. [5, 3]
    dtype        : KarrayType,
    backing      : HwComponentType,   // one of KARRAY_BACKINGS (Reg / Wire)
    backing_hcps : Vec<HcpIdent>,     // index = flat * field_count + field_idx
}

impl Default for Karray {
    fn default() -> Self {
        Self {
            ident       : CcpIdent::new(CcpType::Karray, false, ""),
            shape       : Vec::new(),
            dtype       : KarrayType::default(),
            backing     : HwComponentType::Reg,
            backing_hcps: Vec::new(),
        }
    }
}

impl Karray {
    /// Build a Karray of the given `shape` whose element is the record `fields`,
    /// materialising the backing hardware immediately.
    pub fn new(
        is_user_com: bool,
        name       : &str,
        shape      : Vec<usize>,
        fields     : Vec<(String, i32)>,
        backing    : HwComponentType,
        arena      : &mut ModelArena,
    ) -> Self {
        assert!(KARRAY_BACKINGS.contains(&backing),
            "Karray '{name}': backing must be Reg or Wire, got {backing:?}");
        assert!(!shape.is_empty(), "Karray '{name}': shape must have at least one dimension");
        let total = shape.iter().product::<usize>();
        assert!(total > 0, "Karray '{name}': every dimension must be positive");

        let dtype = KarrayType::new(fields);
        assert!(dtype.field_count() > 0, "Karray '{name}': element must have at least one field");

        let mut backing_hcps = Vec::with_capacity(total * dtype.field_count());
        for flat in 0..total {
            for field in dtype.get_fields() {
                let nm = format!("{name}_E{flat}_{}", field.get_name());
                let hcp_i = match backing {
                    HwComponentType::Reg => arena.make_reg (false, &nm, field.get_width()),
                    _                    => arena.make_wire(false, &nm, field.get_width()),
                };
                backing_hcps.push(hcp_i);
            }
        }

        Self {
            ident: CcpIdent::new(CcpType::Karray, is_user_com, name),
            shape,
            dtype,
            backing,
            backing_hcps,
        }
    }

    // ---- accessors ---------------------------------------------------------
    pub fn get_ccp_ident    (&self)     -> CcpIdent           { self.ident                  }
    pub fn get_ccp_ident_mut(&mut self) -> &mut CcpIdent      { &mut self.ident             }
    pub fn get_shape        (&self)     -> &Vec<usize>        { &self.shape                 }
    pub fn get_dim_count    (&self)     -> usize              { self.shape.len()            }
    pub fn get_backing      (&self)     -> HwComponentType    { self.backing                }
    pub fn get_fields       (&self)     -> &Vec<KarrayField>  { self.dtype.get_fields()     }
    pub fn field_count      (&self)     -> usize              { self.dtype.field_count()    }

    /// Row-major flatten of a full multi-dimensional index, with dim-count and
    /// bounds asserts (`shape [5,3]`, index `[2,1]` → `2*3 + 1 = 7`).
    pub fn flat_index(&self, indices: &[usize]) -> usize {
        assert_eq!(indices.len(), self.shape.len(),
            "Karray: expected {} indices, got {}", self.shape.len(), indices.len());
        let mut flat = 0usize;
        for (&dim, &idx) in self.shape.iter().zip(indices.iter()) {
            assert!(idx < dim, "Karray: index {idx} out of bounds for dimension of size {dim}");
            flat = flat * dim + idx;
        }
        flat
    }

    /// Position of a named field, or `None` if no such field.
    pub fn field_index(&self, name: &str) -> Option<usize> {
        self.dtype.get_fields().iter().position(|f| f.get_name() == name)
    }

    /// The backing HCP of one field at a fully-pinned coordinate — the leaf every
    /// read and write resolves to. Pure lookup, no hardware is created.
    pub(crate) fn element_hcp(&self, coord: &[usize], field_idx: usize) -> HcpIdent {
        let nf = self.field_count();
        assert!(field_idx < nf, "Karray: field index {field_idx} out of range (have {nf})");
        self.backing_hcps[self.flat_index(coord) * nf + field_idx]
    }

    /// Whether the backing is clocked (reg → `|=`) or combinational (wire → `*=`).
    pub fn is_clocked(&self) -> bool {
        matches!(self.backing, HwComponentType::Reg)
    }
}

impl CcpBase for Karray {
    // Backing HCPs are plain registered components; their reset/default events are
    // built by the module build pass. A Karray has no internal graph to wire.
    fn build(&mut self, _arena: &mut ModelArena) {}
    fn set_ccp_ident(&mut self, ident: CcpIdent) { *self.get_ccp_ident_mut() = ident; }
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_karray(*self); }
}

impl Identifiable for Karray {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

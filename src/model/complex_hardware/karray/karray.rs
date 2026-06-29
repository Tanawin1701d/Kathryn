use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::complex_hardware::common::ccp_base::CcpBase;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::complex_hardware::karray::karray_meta::{index_width_for, KarrayField, KarrayType};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::model_arena::ModelArena;

// Which kinds of hardware may back a Karray's elements (a subset of HwComponentType):
// Reg/Wire materialise one HCP per element; MemBlock materialises one addressable MemBlk.
pub const KARRAY_BACKINGS: [HwComponentType; 3] =
    [HwComponentType::Reg, HwComponentType::Wire, HwComponentType::MemBlock];

// ---- Karray -----------------------------------------------------------------

/// A typed multi-dimensional array — a complex component property (CCP). Each
/// field of each element is its **own** HCP (not a bit-slice of a packed element):
/// Reg/Wire materialise `total * field_count` per-field HCPs (row-major element,
/// then field), and MemBlock materialises one MemBlk per field (depth = element
/// count, width = field width). Indexing + field selection resolves to that field's
/// HCP so the usual `|=` / `*=` path applies. Backing HCPs are fixed at
/// construction; there is no internal graph to wire, so `build` is a no-op.
pub struct Karray {
    ident        : CcpIdent,
    shape        : Vec<usize>,        // e.g. [5, 3]
    dtype        : KarrayType,
    backing      : HwComponentType,   // one of KARRAY_BACKINGS (Reg / Wire / MemBlock)
    // Reg/Wire: one HCP per (element, field), index = flat * field_count + field_idx.
    // MemBlock: one MemBlk per field, index = field_idx.
    backing_hcps : Vec<HcpIdent>,
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
    /// Build a Karray of the given `shape` whose element is the packed record
    /// `fields`, materialising the backing hardware immediately (mirrors how
    /// `Arb::new` makes the wires it owns up front).
    pub fn new(
        is_user_com: bool,
        name       : &str,
        shape      : Vec<usize>,
        fields     : Vec<(String, i32)>,
        backing    : HwComponentType,
        arena      : &mut ModelArena,
    ) -> Self {
        assert!(KARRAY_BACKINGS.contains(&backing),
            "Karray '{name}': backing must be Reg, Wire, or MemBlock, got {backing:?}");
        assert!(!shape.is_empty(), "Karray '{name}': shape must have at least one dimension");
        let total = shape.iter().product::<usize>();
        assert!(total > 0, "Karray '{name}': every dimension must be positive");

        let dtype      = KarrayType::new(fields);
        let elem_width = dtype.get_elem_width();
        assert!(elem_width > 0, "Karray '{name}': element must have at least one field");

        /// create hardware component

        let mut backing_hcps = Vec::new();
        match backing {
            HwComponentType::Reg => {
                for flat in 0..total {
                    for field in dtype.get_fields() {
                        let nm = format!("{name}_E{flat}_{}", field.get_name());
                        backing_hcps.push(arena.make_reg(false, &nm, field.get_width()));
                    }
                }
            }
            HwComponentType::Wire => {
                for flat in 0..total {
                    for field in dtype.get_fields() {
                        let nm = format!("{name}_E{flat}_{}", field.get_name());
                        backing_hcps.push(arena.make_wire(false, &nm, field.get_width()));
                    }
                }
            }
            HwComponentType::MemBlock => {
                let iw = index_width_for(total);
                for field in dtype.get_fields() {
                    let nm = format!("{name}_{}_MEM", field.get_name());
                    backing_hcps.push(arena.make_mem_blk(false, &nm, field.get_width(), iw));
                }
            }
            _ => unreachable!("backing validated against KARRAY_BACKINGS above"),
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
    pub fn get_ccp_ident    (&self)     -> CcpIdent           { self.ident                          }
    pub fn get_ccp_ident_mut(&mut self) -> &mut CcpIdent      { &mut self.ident                     }
    pub fn get_shape        (&self)     -> &Vec<usize>        { &self.shape                         }
    pub fn get_dim_size     (&self)     -> usize              { self.shape.len()                    }
    pub fn get_backing      (&self)     -> HwComponentType    { self.backing                        }
    pub fn get_elem_width   (&self)     -> i32                { self.dtype.get_elem_width()         }
    pub fn get_fields       (&self)     -> &Vec<KarrayField>  { self.dtype.get_fields()             }
    pub fn get_backing_hcps (&self)     -> &Vec<HcpIdent>     { &self.backing_hcps                  }
    pub fn total_elems      (&self)     -> usize              { self.shape.iter().product()         }
    pub fn index_width      (&self)     -> i32                { index_width_for(self.total_elems()) }
    pub fn field_count      (&self)     -> usize              { self.dtype.field_count()            }

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

    /// Whether this Karray's fields are clocked (reg/memblock → `|=`) or
    /// combinational (wire → `*=`). Uniform across all fields.
    pub fn is_clocked(&self) -> bool {
        matches!(self.backing, HwComponentType::Reg | HwComponentType::MemBlock)
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

use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::complex_hardware::common::ccp_base::CcpBase;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// Which kinds of hardware may back a Karray's elements (a subset of HwComponentType):
// Reg/Wire materialise one HCP per element; MemBlock materialises one addressable MemBlk.
pub const KARRAY_BACKINGS: [HwComponentType; 3] =
    [HwComponentType::Reg, HwComponentType::Wire, HwComponentType::MemBlock];

// ---- KarrayField / KarrayType -----------------------------------------------

/// One named field of a Karray element. Each field is materialised as its own
/// HCP (not a bit-slice of a packed element), so only its name and width matter.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct KarrayField {
    name  : String,
    width : i32,
}

impl KarrayField {
    pub fn get_name (&self) -> &str { &self.name }
    pub fn get_width(&self) -> i32  { self.width }
}

/// The record layout of a Karray element: an ordered list of fields plus the
/// total element width (sum of field widths, used for whole-element assignment).
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct KarrayType {
    fields    : Vec<KarrayField>,
    elem_width: i32,
}

impl KarrayType {
    /// Build a layout from `(name, width)` pairs, in declaration order.
    pub fn new(fields: Vec<(String, i32)>) -> Self {
        let mut elem_width = 0;
        let mut out        = Vec::with_capacity(fields.len());
        for (name, width) in fields {
            assert!(width > 0, "KarrayType: field '{name}' width must be positive, got {width}");
            elem_width += width;
            out.push(KarrayField { name, width });
        }
        Self { fields: out, elem_width }
    }

    pub fn get_elem_width(&self)         -> i32                { self.elem_width    }
    pub fn field_count   (&self)         -> usize              { self.fields.len()  }
    pub fn get_fields    (&self)         -> &Vec<KarrayField>  { &self.fields       }
    pub fn field_width   (&self, i: usize) -> i32              { self.fields[i].width }
}

// ---- index width helper -----------------------------------------------------

/// Minimum address width (in bits) needed to index `total` elements; never 0.
fn index_width_for(total: usize) -> i32 {
    let mut w = 1;
    while (1usize << w) < total { w += 1; }
    w as i32
}

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

        let _ = elem_width;   // total packed width kept for whole-element assignment
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
    pub fn get_backing      (&self)     -> HwComponentType    { self.backing                        }
    pub fn get_elem_width   (&self)     -> i32                { self.dtype.get_elem_width()         }
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

    /// Resolve a static index + field selection to that field's own HCP, the
    /// target of `|=` / `*=`. Reg/Wire return the per-(element,field) HCP directly;
    /// MemBlock builds a constant-address read/write MemEle on the field's block.
    pub fn resolve_field_hcp(&self, indices: &[usize], field_idx: usize, is_read: bool, arena: &mut ModelArena) -> HcpIdent {
        let nf = self.dtype.field_count();
        assert!(field_idx < nf, "Karray: field index {field_idx} out of range (have {nf})");
        let flat = self.flat_index(indices);
        match self.backing {
            HwComponentType::MemBlock => {
                let blk_i  = self.backing_hcps[field_idx];
                let fwidth = self.dtype.field_width(field_idx);
                let iw     = self.index_width();
                let base   = self.ident.get_global_name().to_string();
                let addr_i = arena.make_val(false, &format!("{base}_ADDR{flat}_F{field_idx}"), iw, flat as u64);
                arena.make_mem_ele(false, &format!("{base}_OP{flat}_F{field_idx}"), blk_i, addr_i, fwidth, is_read)
            }
            // Reg / Wire: the per-(element,field) HCP is the destination directly.
            _ => self.backing_hcps[flat * nf + field_idx],
        }
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

    /// Whole-element assignment: split the packed `src` across the per-field HCPs
    /// (field f at the source's LSB-cumulative offset). Drives one basic assign per
    /// field, so callers keep no field layout of their own.
    pub fn assign_element(&self, indices: &[usize], src_i: HcpIdent, src_slice: Slice, arena: &mut ModelArena) {
        let mut off = 0;
        for field_idx in 0..self.dtype.field_count() {
            let width    = self.dtype.field_width(field_idx);
            let des_i    = self.resolve_field_hcp(indices, field_idx, false, arena);
            let sub      = Slice::new(src_slice.start + off, src_slice.start + off + width);
            arena.gen_basic_assign(des_i, src_i, None, sub);
            off += width;
        }
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

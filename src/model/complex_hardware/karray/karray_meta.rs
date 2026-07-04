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

    // Two fields pair up (for karray-to-karray assignment) on exact name + width.
    pub fn matches(&self, other: &KarrayField) -> bool {
        self.name == other.name && self.width == other.width
    }
}

/// The record layout of a Karray element: an ordered list of fields. The total
/// element width is summed on demand from the fields (single source of truth).
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct KarrayType {
    fields: Vec<KarrayField>,
}

impl KarrayType {
    /// Build a layout from `(name, width)` pairs, in declaration order.
    pub fn new(fields: Vec<(String, i32)>) -> Self {
        let mut out = Vec::with_capacity(fields.len());
        for (name, width) in fields {
            assert!(width > 0, "KarrayType: field '{name}' width must be positive, got {width}");
            out.push(KarrayField { name, width });
        }
        Self { fields: out }
    }

    pub fn get_elem_width(&self)         -> i32              {  self.fields.iter().map(|f| f.width).sum() }
    pub fn field_count   (&self)         -> usize            {  self.fields.len()   }
    pub fn get_fields    (&self)         -> &Vec<KarrayField>{ &self.fields         }
    pub fn field_width   (&self, i: usize) -> i32            {  self.fields[i].width}
}

// ---- index width helper -----------------------------------------------------

/// Minimum address width (in bits) needed to index `total` elements; never 0.
pub fn index_width_for(total: usize) -> i32 {
    let mut w = 1;
    while (1usize << w) < total { w += 1; }
    w as i32
}

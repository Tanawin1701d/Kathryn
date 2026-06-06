use std::fmt;
use std::hash::{Hash, Hasher};
use crate::common::arena_base::ArenaHandle;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::module::module_ident::ModuleIdent;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
#[repr(usize)]
pub enum HwComponentType {
    #[default]
    // hw component that UE fully support
    Reg                = 0,
    StateReg           = 1,
    CondWaitStateReg   = 2,
    CycleWaitStateReg  = 3,
    CntReg             = 4,
    SyncReg            = 5,
    Wire               = 6,
    Nest               = 7,
    Val                = 8,
    MemBlockIndexer    = 9,
    // hw component that require manual src check
    Expression         = 10,
    // io wire static
    IoWire             = 11,
    // no ue support (MemBlockIndexer handles them all, it is only a agent and grouper for )
    MemBlock           = 12,
}

// Sensitivity class of a component — how it is driven, and therefore how it may
// be assigned. Derived purely from `HwComponentType` (see `sensitive_type`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub enum HcpSensitiveType {
    #[default]
    Clocked,        // sequential element written on a clock edge (reg / mem / sp_reg)
    Combinational,  // combinational driver (wire / io_wire)
    ReadOnly,       // not an assignment destination (val / expression result)
}

impl HcpSensitiveType {
    // Only a clocked element is driven on a clock edge; everything else is not.
    pub fn is_clocked(self) -> bool {
        matches!(self, Self::Clocked)
    }

    pub fn name(self) -> &'static str {
        match self {
            Self::Clocked       => "clocked",
            Self::Combinational => "combinational",
            Self::ReadOnly      => "read_only",
        }
    }
}

impl HwComponentType {
    // To add a new UE type: insert it in the enum before Expression and increment this.
    pub const UE_BOUNDARY    : usize = 10;
    // To add a new manual-dep type: append to HW_TYPES_WITH_MAN_DEP and increment this.
    pub const MAN_DEP_COUNT  : usize = 1;
    // To add a new IO wire type: append to HW_TYPES_IO_WIRE and increment this.
    pub const IO_WIRE_COUNT  : usize = 1;
    // To add a new memory block type: append to HW_TYPES_MEM_BLK and increment this.
    pub const MEM_BLK_COUNT  : usize = 1;
    // Derived — never edit directly; bump the group count above instead.
    pub const COUNT          : usize = Self::UE_BOUNDARY
                                     + Self::MAN_DEP_COUNT
                                     + Self::IO_WIRE_COUNT
                                     + Self::MEM_BLK_COUNT;

    pub fn global_prefix(self) -> &'static str {
        match self {
            Self::Reg               => "REG",
            Self::StateReg          => "SR_ST",
            Self::CondWaitStateReg  => "SR_CDWT",
            Self::CycleWaitStateReg => "SR_CYWT",
            Self::CntReg            => "CNT_REG",
            Self::SyncReg           => "SR_SY",
            Self::Wire              => "WIRE",
            Self::Nest              => "NEST",
            Self::Val               => "VAL",
            Self::MemBlockIndexer   => "MEM_BLOCK_INDEXER",
            Self::Expression        => "EXPR",
            Self::IoWire            => "IO_WIRE",
            Self::MemBlock          => "MEM_BLOCK",
        }
    }

}

impl fmt::Display for HwComponentType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.global_prefix())
    }
}

// Length is enforced to equal UE_BOUNDARY at compile time — if you add a new
// UE type, bump UE_BOUNDARY and append the variant here; a mismatch won't compile.
pub const HW_TYPES_WITH_UE: [HwComponentType; HwComponentType::UE_BOUNDARY] = [
    HwComponentType::Reg             , HwComponentType::StateReg,
    HwComponentType::CondWaitStateReg, HwComponentType::CycleWaitStateReg,
    HwComponentType::CntReg          , HwComponentType::SyncReg,
    HwComponentType::Wire            , HwComponentType::Nest,
    HwComponentType::Val             , HwComponentType::MemBlockIndexer,
];

// Length enforced at compile time — add a new manual-dep type here and bump MAN_DEP_COUNT.
pub const HW_TYPES_WITH_MAN_DEP: [HwComponentType; HwComponentType::MAN_DEP_COUNT] = [
    HwComponentType::Expression,
];

// Length enforced at compile time — add a new IO wire type here and bump IO_WIRE_COUNT.
pub const HW_TYPES_IO_WIRE: [HwComponentType; HwComponentType::IO_WIRE_COUNT] = [
    HwComponentType::IoWire,
];

// Length enforced at compile time — add a new memory block type here and bump MEM_BLK_COUNT.
pub const HW_TYPES_MEM_BLK: [HwComponentType; HwComponentType::MEM_BLK_COUNT] = [
    HwComponentType::MemBlock,
];

// Every variant in discriminant order.
// Length == COUNT is enforced at compile time; COUNT itself is derived from the group counts,
// so adding a variant to any group and bumping its count automatically tightens this check.
// Array concatenation (concat4 etc.) requires generic_const_exprs (nightly-only), so the
// entries are listed manually — but COUNT can no longer silently lie.
pub const ALL_HW_TYPES: [HwComponentType; HwComponentType::COUNT] = [
    // UE group (UE_BOUNDARY entries)
    HwComponentType::Reg             , HwComponentType::StateReg,
    HwComponentType::CondWaitStateReg, HwComponentType::CycleWaitStateReg,
    HwComponentType::CntReg          , HwComponentType::SyncReg,
    HwComponentType::Wire            , HwComponentType::Nest,
    HwComponentType::Val             , HwComponentType::MemBlockIndexer,
    // manual-dep group (MAN_DEP_COUNT entries)
    HwComponentType::Expression,
    // IO wire group (IO_WIRE_COUNT entries)
    HwComponentType::IoWire,
    // memory block group (MEM_BLK_COUNT entries)
    HwComponentType::MemBlock,
];

pub fn all_hw_types() -> impl Iterator<Item = HwComponentType> {
    ALL_HW_TYPES.iter().copied()
}

pub trait HcpIdentifiable: Identifiable {

    fn get_iden_base    (&self)     -> &    IdentBase { self.get_ident_base()            }
    fn get_iden_base_mut(&mut self) -> &mut IdentBase { self.get_ident_base_mut() }
    fn get_ident        (&self)     ->      HcpIdent;
    fn get_ident_mut    (&mut self) -> &mut HcpIdent;

    fn set_arena_handler(&mut self, arena_handler: ArenaHandle) {
        self.set_arena_handle(arena_handler);
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct HcpIdent {
    ident_base       : IdentBase,
    hw_type          : HwComponentType,
    sensitive_type   : HcpSensitiveType,
    master_module_i  : ModuleIdent,
}

impl HcpIdent {
    pub fn new(hw_type       : HwComponentType,
               sensitive_type: HcpSensitiveType,
               is_user_com   : bool,
               name          : &str) -> Self {
        Self {
            ident_base     : IdentBase::new(is_user_com, name),
            hw_type,
            sensitive_type,
            master_module_i: ModuleIdent::default(),
        }
    }

    pub fn get_ident_base(&self) -> &IdentBase      { &self.ident_base }
    pub fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    pub fn get_hw_type        (&self) -> HwComponentType  { self.hw_type        }
    pub fn get_sensitive_type (&self) -> HcpSensitiveType { self.sensitive_type }
    pub fn get_master_module_i(&self) -> ModuleIdent { self.master_module_i }
    pub fn set_master_module_i(&mut self, m: ModuleIdent) { self.master_module_i = m; }
    // ---- helpers ------------------------------------------------------------

    pub fn build_unique_hcp_name(&self) -> String {
        format!("{}_{}_{}", self.hw_type,
                            self.ident_base.get_name(),
                            self.ident_base.get_global_id())
    }
}

impl Hash for HcpIdent {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.get_global_id().hash(state);
    }
}

impl Identifiable for HcpIdent {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    fn build_unique_name (&mut self) -> &str {
        let name = self.build_unique_hcp_name();
        self.ident_base.set_name(&name);
        self.ident_base.get_name()
    }
}
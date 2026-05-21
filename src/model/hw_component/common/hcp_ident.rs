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
    // no ue support (MemBlockIndexer handles them all)
    MemBlock           = 12,
}

impl HwComponentType {
    pub const COUNT          : usize = 13;
    // First discriminant outside the "UE fully support" group.
    // All variants with discriminant < UE_BOUNDARY carry an UpdatePool.
    // To add a new UE type: insert it in the enum before Expression and increment this.
    pub const UE_BOUNDARY    : usize = 10;
    // To add a new manual-dep type: append to HW_TYPES_WITH_MAN_DEP and increment this.
    pub const MAN_DEP_COUNT  : usize = 1;

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
    HwComponentType::Reg,              HwComponentType::StateReg,
    HwComponentType::CondWaitStateReg, HwComponentType::CycleWaitStateReg,
    HwComponentType::CntReg,          HwComponentType::SyncReg,
    HwComponentType::Wire,            HwComponentType::Nest,
    HwComponentType::Val,             HwComponentType::MemBlockIndexer,
];

// Length enforced at compile time — add a new manual-dep type here and bump MAN_DEP_COUNT.
pub const HW_TYPES_WITH_MAN_DEP: [HwComponentType; HwComponentType::MAN_DEP_COUNT] = [
    HwComponentType::Expression,
];

pub trait HcpIdentifiable: Identifiable {

    fn get_iden_base    (&self)     -> &IdentBase     { self.get_ident_base()     }
    fn get_iden_base_mut(&mut self) -> &mut IdentBase { self.get_ident_base_mut() }
    fn get_ident_mut    (&mut self) -> &mut HcpIdent;

    fn set_arena_handler(&mut self, arena_handler: ArenaHandle) {
        self.set_arena_handle(arena_handler);
    }
}


#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct HcpIdent {
    ident_base       : IdentBase,
    hw_type          : HwComponentType,
    master_module_i  : ModuleIdent,
}

impl HcpIdent {
    pub fn new(hw_type  : HwComponentType,
               is_user_com: bool,
               name     : &str) -> Self {
        Self {
            ident_base: IdentBase::new(is_user_com, name),
            hw_type,
            master_module_i: ModuleIdent::default(),
        }
    }

    pub fn get_ident_base(&self) -> &IdentBase      { &self.ident_base }
    pub fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    pub fn get_hw_type   (&self) -> HwComponentType { self.hw_type     }
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
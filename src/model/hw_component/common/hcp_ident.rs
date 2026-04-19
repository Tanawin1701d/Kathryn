use std::fmt;
use crate::model::common::identifier::{IdentBase, Identifiable};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(usize)]
pub enum HwComponentType {
    Reg                = 0,
    StateReg           = 1,
    CondWaitStateReg   = 2,
    CycleWaitStateReg  = 3,
    Wire               = 4,
    Expression         = 5,
    Nest               = 6,
    Module             = 7,
    Val                = 8,
    MemBlock           = 9,
    MemBlockIndexer    = 10,
    CounterReg         = 11,
}

impl HwComponentType {
    pub const COUNT: usize = 12;

    pub fn global_prefix(self) -> &'static str {
        match self {
            Self::Reg               => "REG",
            Self::StateReg          => "SR_ST",
            Self::CondWaitStateReg  => "SR_CDWT",
            Self::CycleWaitStateReg => "SR_CYWT",
            Self::Wire              => "WIRE",
            Self::Expression        => "EXPR",
            Self::Nest              => "NEST",
            Self::Module            => "MODULE",
            Self::Val               => "VAL",
            Self::MemBlock          => "MEM_BLOCK",
            Self::MemBlockIndexer   => "MEM_BLOCK_INDEXER",
            Self::CounterReg        => "CNT_REG",
        }
    }
}

impl fmt::Display for HwComponentType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.global_prefix())
    }
}


#[derive(Clone, Debug, PartialEq, Eq)]
pub struct HcpIdent {
    ident_base : IdentBase,
    hw_type    : HwComponentType,
}

impl HcpIdent {
    pub fn new(hw_type  : HwComponentType,
               is_user_com: bool,
               name     : &str) -> Self {
        Self {
            ident_base: IdentBase::new(is_user_com, name),
            hw_type,
        }
    }
    
    pub fn get_ident_base(&self) -> &IdentBase      { &self.ident_base }
    pub fn get_hw_type   (&self) -> HwComponentType { self.hw_type     }
    // ---- helpers ------------------------------------------------------------

    pub fn build_unique_hcp_name(&self) -> String {
        format!("{}_{}_{}", self.hw_type,
                            self.ident_base.get_name(),
                            self.ident_base.get_global_id())
    }
}
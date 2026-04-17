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
    Box                = 11,
    Intf               = 12,
    PmVal              = 13,
    CounterReg         = 14,
}

impl HwComponentType {
    pub const COUNT: usize = 15;

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
            Self::Box               => "BOX",
            Self::Intf              => "ITF",
            Self::PmVal             => "PMVAL",
            Self::CounterReg        => "CNT_REG",
        }
    }
}

impl fmt::Display for HwComponentType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.global_prefix())
    }
}

pub trait HcpIdentifiable : Identifiable {
    fn get_hcp_ident    (&self)     -> &HcpIdent    ;
    fn get_hcp_ident_mut(&mut self) -> &mut HcpIdent;
}

pub struct HcpIdent {
    ident_base : IdentBase,
    hw_type    : HwComponentType,
}

impl HcpIdent {
    pub fn new(hw_type: HwComponentType, name: &str) -> Self {
        let mut s = Self {
            ident_base: IdentBase::new(name),
            hw_type,
        };
        s
    }

    pub fn get_hw_type(&self) -> HwComponentType { self.hw_type }

    pub fn build_unique_hcp_name(&self) -> String {
        format!("{}_{}_{}", self.hw_type,
                            self.ident_base.get_name(),
                            self.ident_base.get_global_id())
    }
}
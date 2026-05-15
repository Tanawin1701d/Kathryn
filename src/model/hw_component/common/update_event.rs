pub const DEFAULT_UE_PRI_USER          : i32 = 10;
pub const DEFAULT_UE_PRI_INTERNAL_MAX  : i32 = 100;
pub const DEFAULT_UE_PRI_INTERNAL_MIN  : i32 = 50;
pub const DEFAULT_UE_PRI_RST           : i32 = i32::MAX;
pub const DEFAULT_UE_PRI_MIN           : i32 = 0;
pub const DEFAULT_UE_SUB_PRIORITY_USER : u64 = 0;

use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub enum UeType {
    Basic  = 0,
    Grp    = 1,
    Cond   = 2,
    Switch = 3,
    #[default]
    Untype = 4,
}

impl UeType {
    pub fn prefix(self) -> &'static str {
        match self {
            UeType::Basic  => "UE_BASIC",
            UeType::Grp    => "UE_GRP",
            UeType::Cond   => "UE_COND",
            UeType::Switch => "UE_SWITCH",
            UeType::Untype => "UE_UNTYPE",
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct UeCommon {
    ue_type      : UeType,
    priority     : i32,
    sub_priority : u64,
    clk_mode     : ClockMode,
}

impl Default for UeCommon {
    fn default() -> Self {
        Self {
            ue_type      : UeType::Untype,
            priority     : DEFAULT_UE_PRI_USER,
            sub_priority : DEFAULT_UE_SUB_PRIORITY_USER,
            clk_mode     : ClockMode::ClkUnused,
        }
    }
}

impl UeCommon {
    pub fn new(ue_type: UeType, priority: i32, clk_mode: ClockMode) -> Self {
        Self { ue_type, priority, clk_mode, ..Default::default() }
    }

    pub fn get_priority    (&self) -> i32       { self.priority     }
    pub fn get_sub_priority(&self) -> u64       { self.sub_priority }
    pub fn get_clk_mode    (&self) -> ClockMode { self.clk_mode     }
    pub fn get_ue_type     (&self) -> UeType    { self.ue_type      }
}

pub trait HasUeCommon {
    fn get_ue_common    (&self)     -> &UeCommon;
    fn get_ue_common_mut(&mut self) -> &mut UeCommon;
}

pub trait UpdatingEvent: HasUeCommon {
    fn get_type         (&self) -> UeType     { self.get_ue_common().ue_type      }
    fn get_priority     (&self) -> i32        { self.get_ue_common().priority     }
    fn get_sub_priority (&self) -> u64        { self.get_ue_common().sub_priority }
    fn get_clk_mode     (&self) -> ClockMode  { self.get_ue_common().clk_mode     }
    fn is_joinable      (&self, rhs: &dyn UpdatingEvent) -> bool {
        (self.get_priority() == rhs.get_priority()) &&
        (self.get_clk_mode() == rhs.get_clk_mode())
    }
    fn set_priority    (&mut self, priority: i32)       { self.get_ue_common_mut().priority     = priority;     }
    fn set_sub_priority(&mut self, sub_priority: u64)   { self.get_ue_common_mut().sub_priority = sub_priority; }
    fn set_clk_mode    (&mut self, clk_mode: ClockMode) { self.get_ue_common_mut().clk_mode     = clk_mode;     }

    fn is_leaf(&self) -> bool;
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena);
}


/*
    UPDATE EVENT BASIC
*/
#[derive(Debug, PartialEq, Eq)]
pub struct UeBasic {
    ident    : UpdateEventIdent,
    ue_common: UeCommon,
    srci     : HcpIdent,
    des_slice: Slice,
    src_slice: Slice,
}

impl UeBasic {
    pub fn new(srci: HcpIdent, des_slice: Slice, src_slice: Slice) -> Self {
        Self {
            ident    : UpdateEventIdent::new(IdentBase::new(false, UeType::Basic.prefix()), UeType::Basic),
            ue_common: UeCommon { ue_type: UeType::Basic, ..Default::default() },
            srci,
            des_slice,
            src_slice,
        }
    }

    pub fn ident      (&self) -> UpdateEventIdent { self.ident      }
    pub fn ue_common  (&self) -> &UeCommon        { &self.ue_common }
    pub fn get_des_slice(&self) -> &Slice         { &self.des_slice }
    pub fn get_srci_val (&self) -> &HcpIdent      { &self.srci      }
}

impl HasUeCommon for UeBasic {
    fn get_ue_common    (&self)     -> &UeCommon     { &self.ue_common }
    fn get_ue_common_mut(&mut self) -> &mut UeCommon { &mut self.ue_common }
}

impl UpdatingEvent for UeBasic {
    fn is_leaf(&self) -> bool { true }
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_basic(*self);
    }
}

impl Default for UeBasic {
    fn default() -> Self {
        Self {
            ident    : UpdateEventIdent::default(),
            ue_common: UeCommon::default(),
            srci     : HcpIdent::default(),
            des_slice: Slice::default(),
            src_slice: Slice::default(),
        }
    }
}

impl Identifiable for UeBasic {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}


/*
    UPDATE EVENT GROUP
*/
pub struct UeGrp {
    ident    : UpdateEventIdent,
    ue_common: UeCommon,
    sub_stmts: Vec<UpdateEventIdent>,
}

impl UeGrp {
    pub fn new() -> Self {
        Self {
            ident    : UpdateEventIdent::new(IdentBase::new(false, UeType::Grp.prefix()), UeType::Grp),
            ue_common: UeCommon { ue_type: UeType::Grp, ..Default::default() },
            sub_stmts: Vec::new(),
        }
    }

    pub fn ident    (&self) -> UpdateEventIdent   { self.ident      }
    pub fn ue_common(&self) -> &UeCommon          { &self.ue_common }

    pub fn add_sub_stmt(&mut self, stmt: UpdateEventIdent, priority: i32, clk_mode: ClockMode) {
        if self.sub_stmts.is_empty() {
            self.ue_common.priority = priority;
            self.ue_common.clk_mode = clk_mode;
        }
        self.sub_stmts.push(stmt);
    }

    pub fn get_sub_stmts(&self) -> &[UpdateEventIdent] { &self.sub_stmts }
}

impl HasUeCommon for UeGrp {
    fn get_ue_common    (&self)     -> &UeCommon     { &self.ue_common }
    fn get_ue_common_mut(&mut self) -> &mut UeCommon { &mut self.ue_common }
}

impl UpdatingEvent for UeGrp {
    fn is_leaf(&self) -> bool { false }
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_grp(*self);
    }
}

impl Default for UeGrp {
    fn default() -> Self {
        Self {
            ident    : UpdateEventIdent::default(),
            ue_common: UeCommon::default(),
            sub_stmts: Vec::new(),
        }
    }
}

impl Identifiable for UeGrp {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}


/*
    UPDATE EVENT COND
*/
pub struct UeCond {
    ident         : UpdateEventIdent,
    ue_common     : UeCommon,
    is_last_occure: bool,
    conditions    : Vec<Option<HcpIdent>>,
    sub_stmts     : Vec<Option<UpdateEventIdent>>,
}

impl UeCond {
    pub fn new() -> Self {
        Self {
            ident         : UpdateEventIdent::new(IdentBase::new(false, UeType::Cond.prefix()), UeType::Cond),
            ue_common     : UeCommon { ue_type: UeType::Cond, ..Default::default() },
            is_last_occure: false,
            conditions    : Vec::new(),
            sub_stmts     : Vec::new(),
        }
    }

    pub fn ident    (&self) -> UpdateEventIdent { self.ident      }
    pub fn ue_common(&self) -> &UeCommon        { &self.ue_common }

    pub fn add_sub_stmt(&mut self, cond: Option<HcpIdent>, stmt: Option<UpdateEventIdent>, priority: i32, clk_mode: ClockMode) {
        assert!(!self.is_last_occure);
        if cond.is_none() { self.is_last_occure = true; }
        if self.sub_stmts.is_empty() {
            self.ue_common.priority = priority;
            self.ue_common.clk_mode = clk_mode;
        }
        self.conditions.push(cond);
        self.sub_stmts .push(stmt);
    }

    pub fn get_conditions(&self) -> &[Option<HcpIdent>]         { &self.conditions }
    pub fn get_sub_stmts (&self) -> &[Option<UpdateEventIdent>] { &self.sub_stmts  }
}

impl HasUeCommon for UeCond {
    fn get_ue_common    (&self)     -> &UeCommon     { &self.ue_common }
    fn get_ue_common_mut(&mut self) -> &mut UeCommon { &mut self.ue_common }
}

impl UpdatingEvent for UeCond {
    fn is_leaf(&self) -> bool { false }
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_cond(*self);
    }
}

impl Default for UeCond {
    fn default() -> Self {
        Self {
            ident         : UpdateEventIdent::default(),
            ue_common     : UeCommon::default(),
            is_last_occure: false,
            conditions    : Vec::new(),
            sub_stmts     : Vec::new(),
        }
    }
}

impl Identifiable for UeCond {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}


/*
    UPDATE EVENT SWITCH
*/
pub struct UeSwitch {
    ident        : UpdateEventIdent,
    ue_common    : UeCommon,
    is_init_meta : bool,
    state_iden   : HcpIdent,
    sub_stmt_idxs: Vec<i32>,
    sub_stmts    : Vec<Option<UpdateEventIdent>>,
}

impl UeSwitch {
    pub fn new(state_iden: HcpIdent) -> Self {
        Self {
            ident        : UpdateEventIdent::new(IdentBase::new(false, UeType::Switch.prefix()), UeType::Switch),
            ue_common    : UeCommon { ue_type: UeType::Switch, ..Default::default() },
            is_init_meta : false,
            state_iden,
            sub_stmt_idxs: Vec::new(),
            sub_stmts    : Vec::new(),
        }
    }

    pub fn ident    (&self) -> UpdateEventIdent { self.ident      }
    pub fn ue_common(&self) -> &UeCommon        { &self.ue_common }

    pub fn get_match_num  (&self) -> usize     { self.sub_stmts.len() }
    pub fn get_state_iden (&self) -> &HcpIdent { &self.state_iden }

    pub fn get_sub_stmt_match_idx(&self, idx: usize) -> i32 {
        assert!(idx < self.sub_stmt_idxs.len());
        self.sub_stmt_idxs[idx]
    }

    pub fn get_sub_stmt(&self, idx: usize) -> Option<UpdateEventIdent> {
        assert!(idx < self.sub_stmts.len());
        self.sub_stmts[idx]
    }

    pub fn add_sub_stmt(&mut self, match_val: i32, stmt: Option<UpdateEventIdent>, priority: i32, clk_mode: ClockMode) {
        if !self.is_init_meta {
            self.ue_common.priority = priority;
            self.ue_common.clk_mode = clk_mode;
            self.is_init_meta = true;
            
        }
        self.sub_stmt_idxs.push(match_val);
        self.sub_stmts.push(stmt);
    }
}

impl HasUeCommon for UeSwitch {
    fn get_ue_common    (&self)     -> &UeCommon     { &self.ue_common }
    fn get_ue_common_mut(&mut self) -> &mut UeCommon { &mut self.ue_common }
}

impl UpdatingEvent for UeSwitch {
    fn is_leaf(&self) -> bool { false }
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_ue_switch(*self);
    }
}

impl Default for UeSwitch {
    fn default() -> Self {
        Self {
            ident        : UpdateEventIdent::default(),
            ue_common    : UeCommon::default(),
            is_init_meta : false,
            state_iden   : HcpIdent::default(),
            sub_stmt_idxs: Vec::new(),
            sub_stmts    : Vec::new(),
        }
    }
}

impl Identifiable for UeSwitch {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

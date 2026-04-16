pub const DEFAULT_UE_PRI_USER          : i32 = 10;
pub const DEFAULT_UE_PRI_INTERNAL_MAX  : i32 = 100;
pub const DEFAULT_UE_PRI_INTERNAL_MIN  : i32 = 50;
pub const DEFAULT_UE_PRI_RST           : i32 = i32::MAX;
pub const DEFAULT_UE_PRI_MIN           : i32 = 0;
pub const DEFAULT_UE_SUB_PRIORITY_USER : u64 = 0;

use std::rc::Rc;
use crate::common::obj::SPTR;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_read::Readable;
use crate::model::hw_component::common::slice::Slice;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum UeType {
    Basic  = 0,
    Grp    = 1,
    Cond   = 2,
    Switch = 3,
    Untype = 4,
}

struct UeCommon {
    ue_type      : UeType,
    priority     : i32,
    sub_priority : u64,
    clk_mode     : ClockMode,
}

trait HasUeCommon {
    /// ////////////////
    /// virtual function
    /// ////////////////
    fn get_ue_common(&self) -> &UeCommon;
    fn get_ue_common_mut(&mut self) -> &mut UeCommon;
}

pub trait UpdatingEvent: HasUeCommon {

    /// ////////////////
    /// common function
    /// ////////////////

    /// retrival function
    fn get_type    (&self)     -> UeType { self.get_ue_common().ue_type      }
    fn get_priority(&self)     -> i32    { self.get_ue_common().priority     }
    fn get_sub_priority(&self) -> u64    { self.get_ue_common().sub_priority }
    fn get_clk_mode(&self) -> ClockMode  { self.get_ue_common().clk_mode     }
    fn is_joinable(&self, rhs: &dyn UpdatingEvent) -> bool{

        return self.get_priority() == rhs.get_priority() &&
               self.get_clk_mode() == rhs.get_clk_mode()
    }
    /// assign function
    fn set_priority(&mut self, priority: i32) {
        self.get_ue_common_mut().priority = priority;
    }
    fn set_sub_priority(&mut self, sub_priority: u64) {
        self.get_ue_common_mut().sub_priority = sub_priority;
    }
    fn set_clk_mode(&mut self, clk_mode: ClockMode) {
        self.get_ue_common_mut().clk_mode = clk_mode;
    }

    /// ////////////////
    /// virtual function
    /// ////////////////
    fn get_dep          (&self, result_dep: &mut Vec<SPTR<dyn Readable>>);
    fn is_leaf          (&self) -> bool;
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

/*
    UPDATE EVENT BASIC
*/
pub struct UeBasic {
    ue_common: UeCommon,
    src_val: SPTR<dyn Readable>,
    des_slice: Slice,
}

impl UeBasic {
    pub fn new(src_val: SPTR<dyn Readable>, des_slice: Slice) -> Self {
        Self {
            ue_common: UeCommon {
                ue_type: UeType::Basic,
                ..Default::default()
            },
            src_val,
            des_slice,
        }
    }

    pub fn get_des_slice(&self) -> &Slice{&self.des_slice}
    pub fn get_src_val(&self)   -> &SPTR<dyn Readable>{&self.src_val}
}

impl HasUeCommon for UeBasic {
    fn get_ue_common(&self) -> &UeCommon{&self.ue_common}
    fn get_ue_common_mut(&mut self) -> &mut UeCommon{&mut self.ue_common}
}

impl UpdatingEvent for UeBasic{
    fn get_dep(&self, result_dep: &mut Vec<SPTR<dyn Readable>>) {
        result_dep.push(self.src_val.clone());
    }
    fn is_leaf(&self) -> bool{true}
}


/*
    UPDATE EVENT GROUP
*/
pub struct UeGrp {
    ue_common : UeCommon,
    sub_stmts : Vec<SPTR<dyn UpdatingEvent>>,
}

impl UeGrp {
    pub fn new() -> Self {
        Self {
            ue_common : UeCommon { ue_type: UeType::Grp, ..Default::default() },
            sub_stmts : Vec::new(),
        }
    }

    pub fn add_sub_stmt(&mut self, stmt: SPTR<dyn UpdatingEvent>) {
        if self.sub_stmts.is_empty() {
            self.ue_common.priority = stmt.borrow().get_priority();
            self.ue_common.clk_mode = stmt.borrow().get_clk_mode();
        }
        self.sub_stmts.push(stmt);
    }
}

impl HasUeCommon for UeGrp {
    fn get_ue_common(&self)         -> &UeCommon     { &self.ue_common }
    fn get_ue_common_mut(&mut self) -> &mut UeCommon { &mut self.ue_common }
}

impl UpdatingEvent for UeGrp {
    fn get_dep(&self, result_dep: &mut Vec<SPTR<dyn Readable>>) {
        for stmt in &self.sub_stmts {
            stmt.borrow().get_dep(result_dep);
        }
    }
    fn is_leaf(&self) -> bool { false }
}

/*
    UPDATE EVENT COND
*/
pub struct UeCond {
    ue_common      : UeCommon,
    is_last_occure : bool,
    conditions     : Vec<Option<SPTR<dyn Readable>>>,
    sub_stmts      : Vec<SPTR<dyn UpdatingEvent>>,
}

impl UeCond {
    pub fn new() -> Self {
        Self {
            ue_common      : UeCommon { ue_type: UeType::Cond, ..Default::default() },
            is_last_occure : false,
            conditions     : Vec::new(),
            sub_stmts      : Vec::new(),
        }
    }

    pub fn add_sub_stmt(&mut self, cond: Option<SPTR<dyn Readable>>, stmt: SPTR<dyn UpdatingEvent>) {
        assert!(!self.is_last_occure);
        if cond.is_none() {
            // it means thisd is "else" part
            self.is_last_occure = true;
        }
        if self.sub_stmts.is_empty() {
            self.ue_common.priority = stmt.borrow().get_priority();
            self.ue_common.clk_mode = stmt.borrow().get_clk_mode();
        }
        self.conditions.push(cond);
        self.sub_stmts .push(stmt);
    }
}

impl HasUeCommon for UeCond {
    fn get_ue_common(&self)         -> &UeCommon     { &self.ue_common }
    fn get_ue_common_mut(&mut self) -> &mut UeCommon { &mut self.ue_common }
}

impl UpdatingEvent for UeCond {
    fn get_dep(&self, result_dep: &mut Vec<SPTR<dyn Readable>>) {
        for cond in &self.conditions {
            if let Some(c) = cond {
                result_dep.push(c.clone());
            }
        }
        for stmt in &self.sub_stmts {
            stmt.borrow().get_dep(result_dep);
        }
    }
    fn is_leaf(&self) -> bool { false }
}

/*
    UPDATE EVENT SWITCH
*/
pub struct UeSwitch {
    ue_common    : UeCommon,
    is_init_meta : bool,
    state_iden   : SPTR<dyn Readable>,
    sub_stmt_idxs: Vec<i32>,
    sub_stmts    : Vec<Option<SPTR<dyn UpdatingEvent>>>,
}

impl UeSwitch {
    pub fn new(state_iden: SPTR<dyn Readable>) -> Self {
        Self {
            ue_common    : UeCommon { ue_type: UeType::Switch, ..Default::default() },
            is_init_meta : false,
            state_iden,
            sub_stmt_idxs: Vec::new(),
            sub_stmts    : Vec::new(),
        }
    }

    pub fn get_match_num(&self) -> usize { self.sub_stmts.len() }
    pub fn get_state_iden(&self) -> &SPTR<dyn Readable> { &self.state_iden }

    pub fn get_sub_stmt_match_idx(&self, idx: usize) -> i32 {
        assert!(idx < self.sub_stmt_idxs.len());
        self.sub_stmt_idxs[idx]
    }

    pub fn get_sub_stmt(&self, idx: usize) -> Option<SPTR<dyn UpdatingEvent>> {
        assert!(idx < self.sub_stmts.len());
        self.sub_stmts[idx].clone()
    }

    pub fn add_sub_stmt(&mut self, match_val: i32, stmt: Option<SPTR<dyn UpdatingEvent>>) {
        if !self.is_init_meta {
            if let Some(s) = &stmt {
                self.ue_common.priority = s.borrow().get_priority();
                self.ue_common.clk_mode = s.borrow().get_clk_mode();
                self.is_init_meta = true;
            }
        }
        self.sub_stmt_idxs.push(match_val);
        self.sub_stmts.push(stmt);
    }
}

impl HasUeCommon for UeSwitch {
    fn get_ue_common(&self)         -> &UeCommon     { &self.ue_common }
    fn get_ue_common_mut(&mut self) -> &mut UeCommon { &mut self.ue_common }
}

impl UpdatingEvent for UeSwitch {
    fn get_dep(&self, result_dep: &mut Vec<SPTR<dyn Readable>>) {
        result_dep.push(self.state_iden.clone());
        for stmt in &self.sub_stmts {
            if let Some(s) = stmt {
                s.borrow().get_dep(result_dep);
            }
        }
    }
    fn is_leaf(&self) -> bool { false }
}
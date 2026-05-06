use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::module::module_ident::ModuleIdent;

// ---------------------------------------------------------------------------
// ModelStage — mirrors C++ MODEL_STAGE enum.
// ---------------------------------------------------------------------------
#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
#[repr(u8)]
pub enum ModelStage {
    #[default]
    ModelUninit     = 0,
    ModelGlobInited = 1,
    ModelFlowInited = 2,
}

// ---------------------------------------------------------------------------
// Module — container for a hardware module's user-declared components and
// implicit (design-flow built) sp_regs.  Stored in ModelArena.modules and
// referenced through ModuleIdent.  Mirrors C++ kathryn::Module, scoped to
// the subset of components currently ported to Rust.
// ---------------------------------------------------------------------------
#[derive(Default)]
pub struct Module {
    ident                : ModuleIdent,
    is_top_module        : bool,
    md_stage             : ModelStage,

    // implicit sp_regs (built by the design flow); indexed by HwComponentType
    sp_regs              : [Vec<HcpIdent>; HwComponentType::COUNT],

    // user-declared hw components; indexed by HwComponentType
    user_hw              : [Vec<HcpIdent>; HwComponentType::COUNT],

    user_sub_modules     : Vec<ModuleIdent>,
}

impl Module {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        Self {
            ident: ModuleIdent::new(is_user_com, name),
            ..Default::default()
        }
    }

    pub fn mk(name: &str) -> Self { Self::new(true, name) }

    pub fn get_ident(&self) -> ModuleIdent { self.ident }

    // -- top module flag --
    pub fn is_top_module(&self) -> bool { self.is_top_module }
    pub fn set_top_module(&mut self)    { self.is_top_module = true; }

    // -- stage --
    pub fn get_stage(&self) -> ModelStage             { self.md_stage }
    pub fn set_stage(&mut self, stage: ModelStage)    { self.md_stage = stage; }

    // -- sp_reg accessors (implicit, indexed by HwComponentType) --
    pub fn add_sp_reg(&mut self, i: HcpIdent) {
        let t = i.get_hw_type();
        self.sp_regs[t as usize].push(i);
    }
    pub fn get_sp_regs(&self, t: HwComponentType) -> &Vec<HcpIdent> {
        &self.sp_regs[t as usize]
    }

    // -- user-component accessors (explicit, indexed by HwComponentType) --
    pub fn add_user_hw(&mut self, i: HcpIdent) {
        let t = i.get_hw_type();
        self.user_hw[t as usize].push(i);
    }
    pub fn get_user_hw(&self, t: HwComponentType) -> &Vec<HcpIdent> {
        &self.user_hw[t as usize]
    }

    // -- sub-modules --
    pub fn add_user_sub_module(&mut self, i: ModuleIdent)   { self.user_sub_modules.push(i); }
    pub fn get_user_sub_modules(&self) -> &Vec<ModuleIdent> { &self.user_sub_modules }
}

impl Identifiable for Module {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

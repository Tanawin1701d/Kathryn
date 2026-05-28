use std::collections::{HashMap, HashSet};
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable};
use crate::model::model_arena::ModelArena;

pub trait HcpBase: HcpAssignable + HcpIdentifiable {


    // Each concrete HCP type routes itself back to the correct typed arena slot — callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena);

    /// Collect all HcpIdents this component depends on.
    /// Default walks the UpdateEvent pool; Expression overrides to read its operands.
    fn gather_dep_hcps(&self, arena: &mut ModelArena, out: &mut HashSet<HcpIdent>) {
        self.get_update_pool().gather_dep_hcps(arena, out);
    }

    /// Rewrite all HcpIdent dependencies according to `map`.
    /// Default remaps through the UpdateEvent pool; Expression overrides to remap its operands.
    fn remap_dep_hcps(&mut self, map: &HashMap<HcpIdent, HcpIdent>, arena: &mut ModelArena) {
        self.get_update_pool().remap_dep_hcps(map, arena);
    }
}
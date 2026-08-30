use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::{
    ExtSigType, FlowBlock, FlowBlockIdent, FlowBlockType, NodeWrap,
    FlowBlockSeq, FlowBlockPar,
    FlowBlockCond, FlowBlockCondElif,
    FlowBlockZeroCondIf, FlowBlockZeroCondElif,
    FlowBlockZeroSwitch, FlowBlockZeroSwitchCase,
    FlowBlockPick, FlowBlockPickIf,
    FlowBlockWhile, FlowBlockDoWhile, FlowBlockCounterLoop,
    FlowBlockWait,
    FlowBlockPip,
    FlowBlockZync,
};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// One typed CRUD triplet (add / take / replace_back) per flow-block type.
// - fn names are spelled out per row (no `paste` dep for ident concatenation)
// - the assert in take_* pins the ident's FlowBlockType to the slot it indexes
macro_rules! flow_block_crud {
    ($block_struct:ident, $fb_storage:ident, $type_enum      :ident,
     $add_fn      :ident, $take_fn   :ident, $replace_back_fn:ident) => {

        pub fn $add_fn(&mut self, block: $block_struct) -> FlowBlockIdent {
            let h = self.$fb_storage.insert(block);
            self.$fb_storage.get(h).get_base().get_ident()
        }
        pub fn $take_fn(&mut self, ident: FlowBlockIdent) -> $block_struct {
            assert_eq!(ident.get_block_type(), FlowBlockType::$type_enum);
            self.$fb_storage.take(*ident.get_arena_handle())
        }
        pub fn $replace_back_fn(&mut self, block: $block_struct) {
            let h = *block.get_arena_handle();
            self.$fb_storage.replace_back(h, block);
        }

    };
}

impl ModelArena {
    // ---- typed CRUD — one row per flow-block type ----

    flow_block_crud!(FlowBlockSeq                   , flow_block_seqs                 , Sequential    ,
                     add_flow_block_seq             , take_flow_block_seq             , replace_back_flow_block_seq             );
    flow_block_crud!(FlowBlockPar                   , flow_block_pars                 , Parallel      ,
                     add_flow_block_par             , take_flow_block_par             , replace_back_flow_block_par             );
    flow_block_crud!(FlowBlockCond                  , flow_block_conds                , CondIf        ,   // CIF / SIF
                     add_flow_block_cond            , take_flow_block_cond            , replace_back_flow_block_cond            );
    flow_block_crud!(FlowBlockCondElif              , flow_block_cond_elifs           , CondElif      ,   // CSELIF / CSELSE
                     add_flow_block_cond_elif       , take_flow_block_cond_elif       , replace_back_flow_block_cond_elif       );
    flow_block_crud!(FlowBlockZeroCondIf            , flow_block_zero_cond_ifs        , ZeroCondIf    ,   // ZIF master
                     add_flow_block_zero_cond_if    , take_flow_block_zero_cond_if    , replace_back_flow_block_zero_cond_if    );
    flow_block_crud!(FlowBlockZeroCondElif          , flow_block_zero_cond_elifs      , ZeroCondElif  ,   // ZELIF / ZELSE
                     add_flow_block_zero_cond_elif  , take_flow_block_zero_cond_elif  , replace_back_flow_block_zero_cond_elif  );
    flow_block_crud!(FlowBlockZeroSwitch            , flow_block_zero_switches        , ZeroSwitch    ,   // master
                     add_flow_block_zero_switch     , take_flow_block_zero_switch     , replace_back_flow_block_zero_switch     );
    flow_block_crud!(FlowBlockZeroSwitchCase        , flow_block_zero_switch_cases    , ZeroSwitchCase,
                     add_flow_block_zero_switch_case, take_flow_block_zero_switch_case, replace_back_flow_block_zero_switch_case);
    flow_block_crud!(FlowBlockPick                  , flow_block_picks                , Pick          ,   // master
                     add_flow_block_pick            , take_flow_block_pick            , replace_back_flow_block_pick            );
    flow_block_crud!(FlowBlockPickIf                , flow_block_pick_ifs             , PickIf        ,   // pif / pidef branch
                     add_flow_block_pick_if         , take_flow_block_pick_if         , replace_back_flow_block_pick_if         );
    flow_block_crud!(FlowBlockWhile                 , flow_block_whiles               , WhileLoop     ,   // CWHILE / SWHILE
                     add_flow_block_while           , take_flow_block_while           , replace_back_flow_block_while           );
    flow_block_crud!(FlowBlockDoWhile               , flow_block_do_whiles            , DoWhile       ,
                     add_flow_block_do_while        , take_flow_block_do_while        , replace_back_flow_block_do_while        );
    flow_block_crud!(FlowBlockCounterLoop           , flow_block_counter_loops        , CounterLoop   ,
                     add_flow_block_counter_loop    , take_flow_block_counter_loop    , replace_back_flow_block_counter_loop    );
    flow_block_crud!(FlowBlockWait                  , flow_block_waits                , Wait          ,   // SCWAIT / SYWAIT
                     add_flow_block_wait            , take_flow_block_wait            , replace_back_flow_block_wait            );
    flow_block_crud!(FlowBlockPip                   , flow_block_pips                 , Pipeline      ,   // PIP
                     add_flow_block_pip             , take_flow_block_pip             , replace_back_flow_block_pip             );
    flow_block_crud!(FlowBlockZync                  , flow_block_zyncs                , Zync          ,   // ZYNC
                     add_flow_block_zync            , take_flow_block_zync            , replace_back_flow_block_zync            );

    // --- polymorphic take / replace / get ---

    pub fn take_flow_block(&mut self, ident: FlowBlockIdent) -> Box<dyn FlowBlock> {
        match ident.get_block_type() {
            FlowBlockType::Sequential     => Box::new(self.take_flow_block_seq              (ident)),
            FlowBlockType::Parallel       => Box::new(self.take_flow_block_par              (ident)),
            FlowBlockType::CondIf         => Box::new(self.take_flow_block_cond             (ident)),
            FlowBlockType::CondElif       => Box::new(self.take_flow_block_cond_elif        (ident)),
            FlowBlockType::ZeroCondIf     => Box::new(self.take_flow_block_zero_cond_if     (ident)),
            FlowBlockType::ZeroCondElif   => Box::new(self.take_flow_block_zero_cond_elif   (ident)),
            FlowBlockType::ZeroSwitch     => Box::new(self.take_flow_block_zero_switch      (ident)),
            FlowBlockType::ZeroSwitchCase => Box::new(self.take_flow_block_zero_switch_case (ident)),
            FlowBlockType::Pick           => Box::new(self.take_flow_block_pick             (ident)),
            FlowBlockType::PickIf         => Box::new(self.take_flow_block_pick_if          (ident)),
            FlowBlockType::WhileLoop      => Box::new(self.take_flow_block_while            (ident)),
            FlowBlockType::DoWhile        => Box::new(self.take_flow_block_do_while         (ident)),
            FlowBlockType::CounterLoop    => Box::new(self.take_flow_block_counter_loop     (ident)),
            FlowBlockType::Wait           => Box::new(self.take_flow_block_wait             (ident)),
            FlowBlockType::Pipeline       => Box::new(self.take_flow_block_pip              (ident)),
            FlowBlockType::Zync           => Box::new(self.take_flow_block_zync             (ident)),
        }
    }

    pub fn replace_back_flow_block(&mut self, block: Box<dyn FlowBlock>) {
        block.replace_back_into_arena(self);
    }

}

// ---- higher-level flow-block operations (formerly flow_block/arena_ops.rs) ----

impl ModelArena {
    pub fn add_node_to_flow_block(&mut self, block_ident: FlowBlockIdent, node: NcpIdent) {
        let mut block = self.take_flow_block(block_ident);
        block.add_element_in_flow_block(node);
        self.replace_back_flow_block(block);
    }

    pub fn add_sub_flow_block_to_flow_block(
        &mut self,
        parent: FlowBlockIdent,
        child : FlowBlockIdent,
    ) {
        let mut block = self.take_flow_block(parent);
        block.add_sub_flow_block(child);
        self.replace_back_flow_block(block);
    }

    pub fn add_con_flow_block_to_flow_block(
        &mut self,
        parent: FlowBlockIdent,
        child : FlowBlockIdent,
    ) {
        let mut block = self.take_flow_block(parent);
        block.add_con_flow_block(child);
        self.replace_back_flow_block(block);
    }

    pub fn add_ext_signal_to_flow_block(
        &mut self,
        block_ident: FlowBlockIdent,
        int_type   : ExtSigType,
        signal     : HcpIdent,
    ) {
        let mut block = self.take_flow_block(block_ident);
        block.get_base_mut().add_int_signal(int_type, signal);
        self.replace_back_flow_block(block);
    }

    pub fn build_flow_block(&mut self, ident: FlowBlockIdent) {
        let mut block = self.take_flow_block(ident);
        block.build_hw_master(self);
        self.replace_back_flow_block(block);
    }

    pub fn summarize_flow_block(&mut self, ident: FlowBlockIdent) -> NodeWrap {
        let block = self.take_flow_block(ident);
        let wrap  = block.summarize_as_block();
        self.replace_back_flow_block(block);
        wrap
    }
}

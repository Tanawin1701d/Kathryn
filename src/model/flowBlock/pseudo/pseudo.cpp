//
// Created by tanawin on 1/12/25.
//

#include "pseudo.h"
#include "model/controller/controller.h"


namespace kathryn{

FlowBlockPseudo::FlowBlockPseudo(AsmNode* plainNode):
            FlowBlockBase(PSUEDO_BLOCK,
                  {
                  {FLOW_ST_BASE_STACK},
                  FLOW_JO_EXT_FLOW,
                  true
                  }),
            _plainNode(plainNode){

        assert(plainNode != nullptr);
        FlowBlockPseudo::on_attach_block();
        FlowBlockBase::add_basic_node(plainNode);
        FlowBlockPseudo::on_detach_block();
    }

    FlowBlockPseudo::~FlowBlockPseudo() {}


    void FlowBlockPseudo::add_basic_node(Node* node) {
        assert(false);
    }

    void FlowBlockPseudo::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockPseudo::add_con_flow_block(FlowBlockBase *fb){
        assert(false);
    }

    void FlowBlockPseudo::add_intr_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_intr_start(), "start interrupt can start in Pseudo block");
        mf_assert(!is_there_intr_rst(), "start interrupt can reset in Pseudo block");
    }

    NodeWrap *FlowBlockPseudo::sumarize_block() {
        assert(false);
    }

    void FlowBlockPseudo::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPseudo::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPseudo::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockPseudo::get_md_describe() {
        assert(false);
    }

    void FlowBlockPseudo::add_md_log(MdLogVal *mdLogVal) {
        assert(false);
    }

    std::vector<AsmNode*> FlowBlockPseudo::extract(){
        return {_plainNode};
    }


}
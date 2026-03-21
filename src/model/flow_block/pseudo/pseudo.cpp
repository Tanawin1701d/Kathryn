//
// Created by tanawin on 1/12/25.
//

#include "pseudo.h"
#include "model/controller/controller.h"


namespace kathryn{

FlowBlockPseudo::FlowBlockPseudo(AsmNode* plain_node):
            FlowBlockBase(PSUEDO_BLOCK,
                  {
                  {FLOW_ST_BASE_STACK},
                  FLOW_JO_EXT_FLOW,
                  true
                  }),
            _plainNode(plain_node){

        assert(plain_node != nullptr);
        FlowBlockPseudo::on_attach_block();
        FlowBlockBase::add_element_in_flow_block(plain_node);
        FlowBlockPseudo::on_detach_block();
    }

    FlowBlockPseudo::~FlowBlockPseudo() {}


    void FlowBlockPseudo::add_element_in_flow_block(Node* node) {
        assert(false);
    }

    void FlowBlockPseudo::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(false);
    }

    void FlowBlockPseudo::add_con_flow_block(FlowBlockBase *fb){
        assert(false);
    }

    void FlowBlockPseudo::add_int_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_int_start(), "start interrupt can start in Pseudo block");
        mf_assert(!is_there_int_rst(), "start interrupt can reset in Pseudo block");
    }

    NodeWrap *FlowBlockPseudo::sumarize_block() {
        assert(false);
    }

    void FlowBlockPseudo::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPseudo::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPseudo::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockPseudo::get_md_describe() {
        assert(false);
    }

    void FlowBlockPseudo::add_md_log(MdLogVal *md_log_val) {
        assert(false);
    }

    std::vector<AsmNode*> FlowBlockPseudo::extract(){
        return {_plainNode};
    }


}
//
// Created by tanawin on 2/2/2567.
//

#include "zelif.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockZELIF::FlowBlockZELIF(Operable& cond):
            FlowBlockBase(ZELIF,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_CON_FLOW,
                                  false
                          }){

        cur_cond = purify_condition(&cond);
    }

    FlowBlockZELIF::FlowBlockZELIF():
            FlowBlockBase(ZELSE,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_CON_FLOW,
                                  false
                          }){

        cur_cond = nullptr;
    }

    Operable *FlowBlockZELIF::get_purified_cur_cond_ptr() {
        return cur_cond;
    }

    FlowBlockZELIF::~FlowBlockZELIF() {
        //_basicNodes.clear();
    }

    void FlowBlockZELIF::add_element_in_flow_block(Node* node) {
        assert(node != nullptr);
        // if (cur_cond != nullptr) {
        //     assert(node->get_node_type() == ASM_NODE);
        //     ((AsmNode*)node)->add_pre_condition(cur_cond, BITWISE_AND);
        // }
        // FlowBlockBase::add_element_in_flow_block(node);
        assert(node->get_node_type() == ASM_NODE);
        AsmNode* casted_node = (AsmNode*)node;
        try_add_or_create_asm_meta(casted_node, _assignMetas, cur_cond);
        add_abandon_node(node);
    }

    void FlowBlockZELIF::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(false);
    }

    void FlowBlockZELIF::add_con_flow_block(FlowBlockBase *fb) {
        assert(false);
    }

    void FlowBlockZELIF::add_int_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_int_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_int_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap* FlowBlockZELIF::sumarize_block() {assert(false);}

    void FlowBlockZELIF::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZELIF::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZELIF::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZELIF::get_md_describe() {
        assert(false);
    }

    void FlowBlockZELIF::add_md_log(MdLogVal *md_log_val) {
        assert(false);
    }

    void FlowBlockZELIF::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockZELIF::do_post_function() {
        on_detach_block();
    }

    std::vector<ZifClassAsm*> FlowBlockZELIF::get_class_ass_metas(){
        return _assignMetas;
    }

}

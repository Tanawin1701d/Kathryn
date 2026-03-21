//
// Created by tanawin on 10/1/26.
//

#include "model/controller/controller.h"
#include "zcase.h"

namespace kathryn{


    FlowBlockZCase::FlowBlockZCase(int case_value):
                FlowBlockBase(ZCASE_BLOCK,
                              {
                                      {FLOW_ST_BASE_STACK},
                                      FLOW_JO_SUB_FLOW,
                                      true
                              }),
                _caseValue(case_value){}

    FlowBlockZCase::~FlowBlockZCase(){
        _basicNodes.clear();
    }

    void FlowBlockZCase::add_element_in_flow_block(Node* node){
        assert(node != nullptr);
        assert(node->get_node_type() == ASM_NODE);
        AsmNode* casted_node = (AsmNode*)node;
        /////// the class may not compatable with the other ASM
        try_add_or_create_asm_meta(casted_node, _assignMetas);
        add_abandon_node(node);
    }


    void FlowBlockZCase::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(false);
    }

    void FlowBlockZCase::add_con_flow_block(FlowBlockBase *fb) {
        assert(false);
    }

    void FlowBlockZCase::add_int_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_int_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_int_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap* FlowBlockZCase::sumarize_block() {assert(false);}

    void FlowBlockZCase::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZCase::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZCase::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZCase::get_md_describe() {
        assert(false);
    }

    void FlowBlockZCase::add_md_log(MdLogVal *md_log_val) {
        assert(false);
    }

    void FlowBlockZCase::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockZCase::do_post_function() {
        on_detach_block();
    }

    std::vector<ClassAssignMeta*> FlowBlockZCase::get_class_ass_metas(){return _assignMetas;}

    int FlowBlockZCase::get_case_id() const{return _caseValue;}



}
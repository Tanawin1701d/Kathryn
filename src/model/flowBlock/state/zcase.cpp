//
// Created by tanawin on 10/1/26.
//

#include "model/controller/controller.h"
#include "zcase.h"

namespace kathryn{


    FlowBlockZCase::FlowBlockZCase(int caseValue):
                FlowBlockBase(ZCASE_BLOCK,
                              {
                                      {FLOW_ST_BASE_STACK},
                                      FLOW_JO_SUB_FLOW,
                                      true
                              }),
                _caseValue(caseValue){}

    FlowBlockZCase::~FlowBlockZCase(){
        _basic_nodes.clear();
    }

    void FlowBlockZCase::add_basic_node(Node* node){
        assert(node != nullptr);
        assert(node->get_node_type() == ASM_NODE);
        AsmNode* castedNode = (AsmNode*)node;
        /////// the class may not compatable with the other ASM
        tryAddOrCreateAsmMeta(castedNode, _assignMetas);
        add_abandon_node(node);
    }


    void FlowBlockZCase::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZCase::add_con_flow_block(FlowBlockBase *fb) {
        assert(false);
    }

    void FlowBlockZCase::add_intr_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_intr_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_intr_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap* FlowBlockZCase::sumarize_block() {assert(false);}

    void FlowBlockZCase::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZCase::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZCase::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZCase::get_md_describe() {
        assert(false);
    }

    void FlowBlockZCase::add_md_log(MdLogVal *mdLogVal) {
        assert(false);
    }

    void FlowBlockZCase::doPreFunction() {
        on_attach_block();
    }

    void FlowBlockZCase::doPostFunction() {
        on_detach_block();
    }

    std::vector<ClassAssignMeta*> FlowBlockZCase::getClassAssMetas(){return _assignMetas;}

    int FlowBlockZCase::getCaseId() const{return _caseValue;}



}
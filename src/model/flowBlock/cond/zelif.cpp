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

        curCond = purify_condition(&cond);
    }

    FlowBlockZELIF::FlowBlockZELIF():
            FlowBlockBase(ZELSE,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_CON_FLOW,
                                  false
                          }){

        curCond = nullptr;
    }

    Operable *FlowBlockZELIF::getPurifiedCurCond() {
        return curCond;
    }

    FlowBlockZELIF::~FlowBlockZELIF() {
        //_basicNodes.clear();
    }

    void FlowBlockZELIF::add_basic_node(Node* node) {
        assert(node != nullptr);
        // if (curCond != nullptr) {
        //     assert(node->getNodeType() == ASM_NODE);
        //     ((AsmNode*)node)->addPreCondition(curCond, BITWISE_AND);
        // }
        // FlowBlockBase::addElementInFlowBlock(node);
        assert(node->get_node_type() == ASM_NODE);
        AsmNode* castedNode = (AsmNode*)node;
        tryAddOrCreateAsmMeta(castedNode, _assignMetas, curCond);
        add_abandon_node(node);
    }

    void FlowBlockZELIF::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZELIF::add_con_flow_block(FlowBlockBase *fb) {
        assert(false);
    }

    void FlowBlockZELIF::add_intr_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_intr_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_intr_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap* FlowBlockZELIF::sumarize_block() {assert(false);}

    void FlowBlockZELIF::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZELIF::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZELIF::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZELIF::get_md_describe() {
        assert(false);
    }

    void FlowBlockZELIF::add_md_log(MdLogVal *mdLogVal) {
        assert(false);
    }

    void FlowBlockZELIF::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockZELIF::do_post_function() {
        on_detach_block();
    }

    std::vector<ZifClassAsm*> FlowBlockZELIF::getClassAssMetas(){
        return _assignMetas;
    }

}

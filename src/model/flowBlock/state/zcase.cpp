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
        _basicNodes.clear();
    }

    void FlowBlockZCase::addElementInFlowBlock(Node* node){
        assert(node != nullptr);
        assert(node->get_node_type() == ASM_NODE);
        AsmNode* castedNode = (AsmNode*)node;
        /////// the class may not compatable with the other ASM
        tryAddOrCreateAsmMeta(castedNode, _assignMetas);
        addAbandonNode(node);
    }


    void FlowBlockZCase::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZCase::addConFlowBlock(FlowBlockBase *fb) {
        assert(false);
    }

    void FlowBlockZCase::addIntSignal(INT_TYPE type, Operable* signal){
        mf_assert(!isThereIntStart(), "start interrupt can start in zblock");
        mf_assert(!isThereIntRst(), "start interrupt can reset in zblock");
    }

    NodeWrap* FlowBlockZCase::sumarizeBlock() {assert(false);}

    void FlowBlockZCase::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZCase::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZCase::buildHwComponent() {
        assert(false);
    }

    std::string FlowBlockZCase::get_md_describe() {
        assert(false);
    }

    void FlowBlockZCase::add_md_log(MdLogVal *mdLogVal) {
        assert(false);
    }

    void FlowBlockZCase::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockZCase::doPostFunction() {
        onDetachBlock();
    }

    std::vector<ClassAssignMeta*> FlowBlockZCase::getClassAssMetas(){return _assignMetas;}

    int FlowBlockZCase::getCaseId() const{return _caseValue;}



}
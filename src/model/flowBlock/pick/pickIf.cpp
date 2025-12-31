//
// Created by tanawin on 7/8/2024.
//

#include "pickIf.h"

#include <model/controller/controller.h>

namespace kathryn{


    FlowBlockPickCond::FlowBlockPickCond(Operable& cond):
    FlowBlockBase(PICK_WHEN,
        {
               {FLOW_ST_BASE_STACK},
               FLOW_JO_SUB_FLOW,
               true
        }
    ),
    condition(&cond){}

    FlowBlockPickCond::~FlowBlockPickCond(){
        //// delete resultNodeWrapper; it is derive from inside block do not delete
    }

    void FlowBlockPickCond::addElementInFlowBlock(Node* node){
        assert(false);
    }

    void FlowBlockPickCond::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    void FlowBlockPickCond::addConFlowBlock(FlowBlockBase* conBlock){
        assert(false);
    }

    NodeWrap* FlowBlockPickCond::sumarizeBlock(){
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockPickCond::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockPickCond::onDetachBlock(){
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        /////// no need to set lazy delete
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPickCond::buildHwComponent(){
        assert(_conBlocks.empty());
        assert(!_subBlocks.empty());
        mfAssert(!isThereIntStart(), "start interrupt can start in elif block but can start in if block or block inside elif block");
        ///mfAssert(!isThereIntRst(), "start interrupt can reset in elif block but can start in if block or block inside elif block");
        resultNodeWrapper = _subBlocks[0]->sumarizeBlock();

    }

    void FlowBlockPickCond::doPreFunction(){
        onAttachBlock();
    }

    void FlowBlockPickCond::doPostFunction(){
        onDetachBlock();
    }


}

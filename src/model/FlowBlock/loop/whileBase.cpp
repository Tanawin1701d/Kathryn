//
// Created by tanawin on 6/12/2566.
//

#include "whileBase.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockWhileBase::FlowBlockWhileBase(Operable& condExpr,
                                           FLOW_BLOCK_TYPE fbt):
    _condExpr(&condExpr),
    FlowBlockBase(fbt,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }) {}

    FlowBlockWhileBase::~FlowBlockWhileBase() {
        delete resultNodeWrapper;
        delete exitNode;
    }

    void FlowBlockWhileBase::addElementInFlowBlock(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockWhileBase::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockWhileBase::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockWhileBase::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockWhileBase::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockWhileBase::doPreFunction() {
        onAttachBlock();
    }
    void FlowBlockWhileBase::doPostFunction() {
        onDetachBlock();
    }

}
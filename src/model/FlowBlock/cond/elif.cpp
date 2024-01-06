//
// Created by tanawin on 8/12/2566.
//

#include "elif.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/cond/if.h"


namespace kathryn{

    /** constructor*/
    FlowBlockElif::FlowBlockElif(Operable &cond):
    FlowBlockBase(ELIF),
    _cond(&cond){}

    FlowBlockElif::FlowBlockElif(): FlowBlockBase(ELSE) {}

    FlowBlockElif::~FlowBlockElif(){
        //// elif is just the proxy so do not delete subblock
        subBlocks.clear();
    }

    void FlowBlockElif::addElementInFlowBlock(Node *node) {
        assert(true); //// due to implicit sublock declaration
    }

    void FlowBlockElif::addSubFlowBlock(FlowBlockBase *subBlock) {
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap *FlowBlockElif::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockElif::onAttachBlock() {
        ctrl->on_attach_flowBlock_elif(this);
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitSubBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockElif::onDetachBlock() {
        implicitSubBlock->onDetachBlock();
        ctrl->on_detach_flowBlock_elif(this);
    }

    void FlowBlockElif::buildHwComponent() {
        assert(!subBlocks.empty());
        resultNodeWrapper = subBlocks[0]->sumarizeBlock();
    }

    void FlowBlockElif::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockElif::doPostFunction() {
        onDetachBlock();
    }


}
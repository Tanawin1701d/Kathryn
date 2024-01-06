//
// Created by tanawin on 5/1/2567.
//

#include "cbreak.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockCBreak::FlowBlockCBreak():
    FlowBlockBase(EXITWHILE),
    LoopStMacro() {

    }

    FlowBlockCBreak::~FlowBlockCBreak(){

    }

    void FlowBlockCBreak::addElementInFlowBlock(Node *node) {
        assert(false);
    }

    void FlowBlockCBreak::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    NodeWrap* FlowBlockCBreak::sumarizeBlock() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockCBreak::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockCBreak::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockCBreak::buildHwComponent() {

        /**build internal node*/
        breakNode = new StateNode();
        normExitNode= new DummyNode(&_make<Val>("cbreakDum", 1, "b0"));

        /**build resultNodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(breakNode);
        resultNodeWrap->addForceExitNode(breakNode);
        resultNodeWrap->addExitNode(normExitNode);

    }

    void FlowBlockCBreak::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockCBreak::doPostFunction() {
        onDetachBlock();
    }


}
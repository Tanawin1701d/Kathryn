//
// Created by tanawin on 5/1/2567.
//

#include "cbreak.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockSCBreak::FlowBlockSCBreak():
    FlowBlockBase(EXITWHILE,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_SUB_FLOW,
                          true
                  }),
    LoopStMacro() {

    }

    FlowBlockSCBreak::~FlowBlockSCBreak(){
        delete resultNodeWrap;
        delete breakNode;
        delete normExitNode;
    }

    void FlowBlockSCBreak::addElementInFlowBlock(Node *node) {
        assert(false);
    }

    void FlowBlockSCBreak::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    NodeWrap* FlowBlockSCBreak::sumarizeBlock() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockSCBreak::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockSCBreak::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockSCBreak::buildHwComponent() {

        assert(_conBlocks.empty());
        /**build internal node*/
        breakNode = new StateNode();
        addSysNode(breakNode);
        fillIntResetToNodeIfThere(breakNode);

        normExitNode = new DummyNode(&_make<Val>("cbreakDum", 1, 0));
        addSysNode(normExitNode);
        /////////normExitNode->addDependNode(breakNode);
        normExitNode->assign();

        /**build resultNodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(breakNode);
        resultNodeWrap->addForceExitNode(breakNode);
        resultNodeWrap->addExitNode(normExitNode);

    }

    std::string FlowBlockSCBreak::getMdDescribe(){
        std::string ret;
        ret += "[breakNode is]"+ (breakNode != nullptr ? breakNode->getMdDescribe() : "") + "\n";
        return ret;
    }

    void FlowBlockSCBreak::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("breakNode is " +
                             (breakNode != nullptr ? breakNode->getMdDescribe() : ""));
    }

    void FlowBlockSCBreak::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockSCBreak::doPostFunction() {
        onDetachBlock();
    }
}
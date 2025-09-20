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
    LoopStMacro() {}

    FlowBlockSCBreak::FlowBlockSCBreak(Operable& opr):
            FlowBlockBase(EXITWHILE,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }),
            LoopStMacro(),
            forceExitOpr(&opr){}

    FlowBlockSCBreak::~FlowBlockSCBreak(){
        delete resultNodeWrap;
        delete breakNode;
        delete breakCondNode;
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
        breakNode = new StateNode(getClockMode());
        addSysNode(breakNode);
        fillIntResetToNodeIfThere(breakNode);

        normExitNode = new DummyNode(&makeOprVal("cbreakDum", 1, 0));
        addSysNode(normExitNode);
        /////////normExitNode->addDependNode(breakNode);
        normExitNode->assign();

        /**build resultNodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(breakNode);
        if (forceExitOpr == nullptr){
            resultNodeWrap->addForceExitNode(breakNode);
        }else{
            breakCondNode = new PseudoNode(1, BITWISE_AND);
            breakCondNode->addDependNode(breakNode, forceExitOpr);
            breakCondNode->assign();
            addSysNode(breakCondNode);
            resultNodeWrap->addForceExitNode(breakCondNode);
        }
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
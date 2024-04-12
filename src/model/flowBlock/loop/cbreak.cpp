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
        assert(_interruptNode[INTR_TYPE_START] == nullptr);

        /**build internal node*/
        breakNode = new StateNode();
        fillResetIntEventToNode(breakNode);
        breakNode->setDependStateJoinOp(BITWISE_AND);

        normExitNode = new DummyNode(&_make<Val>("cbreakDum", 1, 0));
        normExitNode->setDependStateJoinOp(BITWISE_AND);
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

    void FlowBlockSCBreak::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();

        bool isStateRunning = false;

        if (breakNode != nullptr){
            breakNode->simStartCurCycle();
            isStateRunning |= breakNode->isBlockOrNodeRunning();
        }
        if (normExitNode != nullptr){
            normExitNode->simStartCurCycle();
            isStateRunning |= normExitNode->isBlockOrNodeRunning();
        }

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }


    }

    void FlowBlockSCBreak::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        if (breakNode != nullptr){
            breakNode->simExitCurCycle();
        }
        if (normExitNode != nullptr){
            normExitNode->simExitCurCycle();
        }

    }


}
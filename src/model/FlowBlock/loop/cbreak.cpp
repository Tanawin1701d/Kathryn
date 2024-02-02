//
// Created by tanawin on 5/1/2567.
//

#include "cbreak.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockCBreak::FlowBlockCBreak():
    FlowBlockBase(EXITWHILE,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_SUB_FLOW,
                          true
                  }),
    LoopStMacro() {

    }

    FlowBlockCBreak::~FlowBlockCBreak(){
        delete resultNodeWrap;
        delete breakNode;
        delete normExitNode;
        FlowBlockBase::~FlowBlockBase();
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

        assert(conBlocks.empty());
        /**build internal node*/
        breakNode = new StateNode();
        breakNode->setDependStateJoinOp(BITWISE_AND);
        normExitNode= new DummyNode(&_make<Val>("cbreakDum", 1, 0));
        normExitNode->setDependStateJoinOp(BITWISE_AND);
        normExitNode->assign();

        /**build resultNodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(breakNode);
        resultNodeWrap->addForceExitNode(breakNode);
        resultNodeWrap->addExitNode(normExitNode);

    }

    std::string FlowBlockCBreak::getMdDescribe(){
        std::string ret;
        ret += "[breakNode is]"+ (breakNode != nullptr ? breakNode->getMdDescribe() : "") + "\n";
        return ret;
    }

    void FlowBlockCBreak::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("breakNode is " +
                             (breakNode != nullptr ? breakNode->getMdDescribe() : ""));
    }

    void FlowBlockCBreak::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockCBreak::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockCBreak::simStartCurCycle() {
        if (isCurCycleSimulated()){
            return;
        }
        setSimStatus();

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

    void FlowBlockCBreak::simExitCurCycle() {
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
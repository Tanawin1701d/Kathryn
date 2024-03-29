//
// Created by tanawin on 8/12/2566.
//

#include "elif.h"
#include "model/controller/controller.h"
#include "model/flowBlock/cond/if.h"


namespace kathryn{

    /** constructor*/
    FlowBlockElif::FlowBlockElif(Operable &cond):
    FlowBlockBase(CSELIF,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_CON_FLOW,
                          false
                  }),
    _cond(&cond){}

    FlowBlockElif::FlowBlockElif():
    FlowBlockBase(CSELSE,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_CON_FLOW,
                          false
                  }) {}

    FlowBlockElif::~FlowBlockElif(){
    };

    void FlowBlockElif::addElementInFlowBlock(Node* node) {
        assert(false); //// due to implicit sublock declaration
    }

    void FlowBlockElif::addSubFlowBlock(FlowBlockBase* subBlock) {
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap *FlowBlockElif::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockElif::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitSubBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockElif::onDetachBlock() {
        implicitSubBlock->onDetachBlock();
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockElif::buildHwComponent() {
        buildSubHwComponent();

        assert(_conBlocks.empty());
        assert(!_subBlocks.empty());
        resultNodeWrapper = _subBlocks[0]->sumarizeBlock();
    }

    void FlowBlockElif::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockElif::doPostFunction() {
        onDetachBlock();
    }

    std::string FlowBlockElif::getMdDescribe() {
        std::string ret;
        ret += "[ " + FlowBlockBase::getMdIdentVal() + " ]\n";
        ret += "[elif implicitFlowBlock]" + implicitSubBlock->getMdDescribe() + "\n";
        return ret;
    }

    void FlowBlockElif::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ Elif " + FlowBlockBase::getMdIdentVal() + " ]");
        addMdLogRecur(mdLogVal);
    }

    void FlowBlockElif::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();

        bool isStateRunning = false;

        if (implicitSubBlock != nullptr){
            implicitSubBlock->simStartCurCycle();
            isStateRunning |= implicitSubBlock->isBlockOrNodeRunning();
        }

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }
    }

    void FlowBlockElif::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        if (implicitSubBlock != nullptr){
            implicitSubBlock->simExitCurCycle();
        }
    }


}
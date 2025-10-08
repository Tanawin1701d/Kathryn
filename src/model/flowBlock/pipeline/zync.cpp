//
// Created by tanawin on 13/9/25.
//

#include "zync.h"

#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockZyncBase::FlowBlockZyncBase(
        SyncMeta& syncMeta, Operable* acceptCond):
    FlowBlockBase(PIPE_BLOCK,
        {
            {FLOW_ST_BASE_STACK, FLOW_ST_PIP_BLK},
            FLOW_JO_SUB_FLOW,
            true

        }),
    _syncMeta(&syncMeta),
    _acceptCond(acceptCond){ createReadySignal();}


    FlowBlockZyncBase::~FlowBlockZyncBase(){
        delete prepSendNode;
        delete exitNode;
        delete resultNodeWrap;
    }

    void FlowBlockZyncBase::createReadySignal(){
        synReadySignal = new expression(1);
        _syncMeta._syncMasterReady = synReadySignal;
    }

    void FlowBlockZyncBase::assignReadySignal(){
        assert(prepSendNode != nullptr);
        (*synReadySignal) = (*prepSendNode->getExitOpr());
    }

    void FlowBlockZyncBase::addSubFlowBlock    (FlowBlockBase* subBlock){
        assert(false);
    }
    void FlowBlockZyncBase::addConFlowBlock    (FlowBlockBase* conBlock){
        assert(false);
    }

    NodeWrap* FlowBlockZyncBase::sumarizeBlock(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockZyncBase::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZyncBase::onDetachBlock(){
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZyncBase::buildHwMaster(){

        for (Operable* holdOpr: _syncMeta.masterHoldSignals){
            assert(holdOpr != nullptr);
            addHoldSignal(holdOpr);
        }
        FlowBlockBase::buildHwMaster();
    }


    void FlowBlockZyncBase::buildHwComponent(){
        assert(_conBlocks.empty());
        assert(_subBlocks.empty());
        assert(_syncMeta->_syncMatched != nullptr);

        /** init all nodes and condition*/
        prepSendNode = new StateNode(getClockMode());
        exitNode     = new PseudoNode(1, BITWISE_AND);

        /** prepSendNode*/
        std::string debugName = "zyncBlk_" + (_syncMeta == nullptr ? "manualCond" : _syncMeta->getName());
        prepSendNode->setInternalIdent("zyncBlk_" + debugName);
        fillIntResetToNodeIfThere(prepSendNode);
        fillHoldToNodeIfThere    (prepSendNode);
            /** assign assignment node*/
        Operable* waitCond = _syncMeta->_syncMatched; ///// we should wait further
        if (_acceptCond != nullptr){ //// accept is condition from user to skip this session
            waitCond = &((*waitCond)&(*_acceptCond));
        }
        prepSendNode->addDependNode(prepSendNode, waitCond);
            /** add slave assignment node*/
        for (auto nd : _basicNodes){
            assert(nd->getNodeType() == ASM_NODE);
            prepSendNode->addSlaveAsmNode((AsmNode*)nd, waitCond);
        }

        /** exit Node*/
        Operable* exitCond = _syncMeta->_syncMatched;
        if (_acceptCond != nullptr){
            exitCond = &((*exitCond)&(*_acceptCond));
        }
        exitNode->addDependNode(prepSendNode, exitCond);

        /** assign node*/
        exitNode->assign();
        /** add system node*/
        addSysNode(prepSendNode);
        addSysNode(exitNode);
        /** resultNode Wrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(prepSendNode);
        resultNodeWrap->addExitNode(exitNode);

        /** assign the ready signal*/
        assignReadySignal();

    }

    void FlowBlockZyncBase::addMdLog(MdLogVal* mdLogVal){
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal(prepSendNode->getMdIdentVal() + " " + prepSendNode->getMdDescribe());
        mdLogVal->addVal(exitNode->getMdIdentVal() + " " + exitNode->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrap->getMdIdentVal() + " " + resultNodeWrap->getMdDescribe());
    }

    void FlowBlockZyncBase::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockZyncBase::doPostFunction(){
        onDetachBlock();
    }

}
//
// Created by tanawin on 13/9/25.
//

#include "zync.h"

#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockZyncBase::FlowBlockZyncBase(SyncMeta& syncMeta):
    FlowBlockBase(PIPE_BLOCK,
        {


        }),
    _syncMeta(syncMeta){}


    FlowBlockZyncBase::~FlowBlockZyncBase(){
        delete prepSendNode;
        delete exitNode;
    }

    void FlowBlockZyncBase::createReadySignal(){
        _syncMeta._syncMasterReady = new expression(1);
    }

    void FlowBlockZyncBase::assignReadySignal(){
        assert(prepSendNode != nullptr);
        (*_syncMeta._syncMasterReady) = (*prepSendNode->getExitOpr());
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

    void FlowBlockZyncBase::buildHwComponent(){
        assert(_conBlocks.empty());
        assert(_subBlocks.empty());
        /**try build the match signal*/
        _syncMeta.tryBuildmatchSignal();
        assert(_syncMeta._syncMatched != nullptr);

        /** init all nodes and condition*/
        prepSendNode = new StateNode();
        exitNode     = new PseudoNode(1, BITWISE_AND);

        /** prepSendNode*/

        prepSendNode->setInternalIdent("zyncBlk_" + _syncMeta.getName());
        fillIntResetToNodeIfThere(prepSendNode);
        fillHoldToNodeIfThere    (prepSendNode);
            /** assign assignment node*/
        Operable* waitCond = _syncMeta._syncMatched; ///// we should wait further
        if (_aceeptCond != nullptr){
            waitCond = &((*waitCond)&(*_aceeptCond));
        }
        prepSendNode->addDependNode(prepSendNode, waitCond);
            /** add slave assignment node*/
        for (auto nd : _basicNodes){
            assert(nd->getNodeType() == ASM_NODE);
            prepSendNode->addSlaveAsmNode((AsmNode*)nd);
        }

        /** exit Node*/
        Operable* exitCond = _syncMeta._syncMatched;
        if (_aceeptCond != nullptr){
            exitCond = &((*exitCond)&(*_aceeptCond));
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

    void FlowBlockZyncBase::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockZyncBase::doPostFunction(){
        onDetachBlock();
    }

}
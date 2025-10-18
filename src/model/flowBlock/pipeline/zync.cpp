//
// Created by tanawin on 13/9/25.
//

#include "zync.h"

#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockZyncBase::FlowBlockZyncBase(Operable* condOnly):
    FlowBlockBase(PIPE_BLOCK,
        {     /////// if there is change, please do not forget to fix the constructor below
            {FLOW_ST_BASE_STACK, FLOW_ST_PIP_BLK},
            FLOW_JO_SUB_FLOW,
            true
        }
    ),
    autoGenSyncMeta(true),
    _syncMeta(new SyncMeta("autoGenSyncMeta_pureCond")),
    _acceptCond(condOnly){
        Val& fallTrueSlave = mOprVal("fallTrueZyncSlave", 1, 1);
        _syncMeta->setSlaveReady(fallTrueSlave);
        _syncMeta->setSlaveFinish(fallTrueSlave);
    }

    FlowBlockZyncBase::FlowBlockZyncBase(
        SyncMeta& syncMeta, Operable* acceptCond):
    FlowBlockBase(PIPE_BLOCK,
        {      /////// if there is change, please do not forget to fix the constructor below
            {FLOW_ST_BASE_STACK, FLOW_ST_PIP_BLK},
            FLOW_JO_SUB_FLOW,
            true

        }),
    _syncMeta(&syncMeta),
    _acceptCond(acceptCond){}

    FlowBlockZyncBase::~FlowBlockZyncBase(){
        if (autoGenSyncMeta){
            delete _syncMeta;
        }
        delete prepSendNode;
        delete exitNode;
        delete resultNodeWrap;
    }

    void FlowBlockZyncBase::assignReadySignal(){
        assert(prepSendNode != nullptr);
        assert(_syncMeta != nullptr);
        //_syncMeta->setMasterReady()
        Operable* ready2Sync = addLogicWithOutput(prepSendNode->getExitOpr(), _acceptCond, BITWISE_AND);
        _syncMeta->setMasterReady(*ready2Sync);
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

        if(_syncMeta != nullptr){
            for (Operable* holdOpr: _syncMeta->masterHoldSignals){
                assert(holdOpr != nullptr);
                addHoldSignal(holdOpr);
            }
            for (Operable* killOpr: _syncMeta->masterKillSignals){
                assert(killOpr != nullptr);
                addIntSignal(INT_RESET, killOpr);
            }
        }
        FlowBlockBase::buildHwMaster();
    }


    void FlowBlockZyncBase::buildHwComponent(){
        assert(_conBlocks.empty());
        assert(_subBlocks.empty());
        //assert(_syncMeta->_syncMatched != nullptr);

        /** init all nodes and condition*/
        prepSendNode = new StateNode(getClockMode());
        exitNode     = new PseudoNode(1, BITWISE_AND);

        /** prepSendNode*/
        std::string debugName = "zyncBlk_" + (_syncMeta == nullptr ? "manualCond" : _syncMeta->getName());
        prepSendNode->setInternalIdent("zyncBlk_" + debugName);
        fillIntResetToNodeIfThere(prepSendNode);
        fillHoldToNodeIfThere    (prepSendNode);
        /** assign assignment node*/
        Operable* readyFinal = nullptr;
        readyFinal = _acceptCond;
        if (_syncMeta != nullptr){
            readyFinal = addLogicWithOutput(&_syncMeta->_syncSlaveReady, readyFinal, BITWISE_AND);
        }
        assert(readyFinal != nullptr);
        Operable* notReadyFinal = &(~(*readyFinal));

        prepSendNode->addDependNode(prepSendNode, notReadyFinal);
            /** add slave assignment node*/
        for (auto nd : _basicNodes){
            assert(nd->getNodeType() == ASM_NODE);
            prepSendNode->addSlaveAsmNode((AsmNode*)nd, readyFinal);
        }

        /** exit Node*/
        exitNode->addDependNode(prepSendNode, readyFinal);
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
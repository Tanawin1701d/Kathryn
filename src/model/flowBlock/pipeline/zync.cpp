//
// Created by tanawin on 13/9/25.
//

#include "zync.h"

#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockZyncBase::FlowBlockZyncBase(
        SyncMeta& syncMeta, Operable* acceptCond):
    FlowBlockBase(PIPE_BLOCK,
        {      /////// if there is change, please do not forget to fix the constructor below
            {FLOW_ST_BASE_STACK, FLOW_ST_PIP_BLK},
            FLOW_JO_SUB_FLOW,
            true

        }),
    _syncMeta(syncMeta),
    _acceptCond(acceptCond){}

    FlowBlockZyncBase::~FlowBlockZyncBase(){

        delete prepSendNode;
        delete exitNode;
        delete resultNodeWrap;
    }

    void FlowBlockZyncBase::assignReadySignal(){
        assert(prepSendNode != nullptr);
        //_syncMeta->setMasterReady()
        Operable* ready2Sync = addLogicWithOutput(prepSendNode->get_exit_opr_ptr(), _acceptCond, BITWISE_AND);
        _syncMeta.setMasterReady(*ready2Sync);
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

        addHoldSignal(&_syncMeta.holdMasterSignal);
        addIntSignal(INT_RESET, &_syncMeta.killMasterSignal);

        FlowBlockBase::buildHwMaster();
    }


    void FlowBlockZyncBase::buildHwComponent(){
        assert(_conBlocks.empty());
        assert(_subBlocks.empty());
        //assert(_syncMeta->_syncMatched != nullptr);

        /** init all nodes and condition*/
        prepSendNode = new StateNode(getClockMode());
        exitNode     = new PseudoNode(1, BITWISE_AND);

        ////// if it auto we have to build the auto trigger for
        if (isAutoActivatePipe()){
            Val& autoActivateSignal = makeOprVal("zync_auto_act_" + _zyncName, 1, 1);
            _syncMeta.setSlaveReady(autoActivateSignal);
            _syncMeta.setSlaveFinish(autoActivateSignal);
        }
        /** prepSendNode*/
        std::string debugName = "zyncBlk_" + _syncMeta.getName();
        prepSendNode->set_internal_ident("zyncBlk_" + debugName);
        fillIntResetToNodeIfThere(prepSendNode);
        fillHoldToNodeIfThere    (prepSendNode);
        /** assign assignment node*/
        Operable* readyFinal = nullptr;
        readyFinal = _acceptCond;
        readyFinal = addLogicWithOutput(&_syncMeta._syncSlaveReady, readyFinal, BITWISE_AND);

        assert(readyFinal != nullptr);
        Operable* notReadyFinal = &(~(*readyFinal));

        prepSendNode->add_depend_node(prepSendNode, notReadyFinal);
            /** add slave assignment node*/
        for (auto nd : _basicNodes){
            assert(nd->get_node_type() == ASM_NODE);
            prepSendNode->addSlaveAsmNode((AsmNode*)nd, readyFinal);
        }

        /** exit Node*/
        exitNode->add_depend_node(prepSendNode, readyFinal);
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

    void FlowBlockZyncBase::add_md_log(MdLogVal* mdLogVal){
        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        mdLogVal->addVal(prepSendNode->get_md_ident_val() + " " + prepSendNode->get_md_describe());
        mdLogVal->addVal(exitNode->get_md_ident_val() + " " + exitNode->get_md_describe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrap->get_md_ident_val() + " " + resultNodeWrap->get_md_describe());
    }

    void FlowBlockZyncBase::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockZyncBase::doPostFunction(){
        onDetachBlock();
    }

}
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

    void FlowBlockZyncBase::add_sub_flow_block    (FlowBlockBase* subBlock){
        assert(false);
    }
    void FlowBlockZyncBase::add_con_flow_block    (FlowBlockBase* conBlock){
        assert(false);
    }

    NodeWrap* FlowBlockZyncBase::sumarize_block(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockZyncBase::on_attach_block(){
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZyncBase::on_detach_block(){
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZyncBase::build_hw_master(){

        add_hold_signal(&_syncMeta.holdMasterSignal);
        add_intr_signal(INT_RESET, &_syncMeta.killMasterSignal);

        FlowBlockBase::build_hw_master();
    }


    void FlowBlockZyncBase::build_hw_component(){
        assert(_con_blocks.empty());
        assert(_sub_blocks.empty());
        //assert(_syncMeta->_syncMatched != nullptr);

        /** init all nodes and condition*/
        prepSendNode = new StateNode(get_clock_mode());
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
        fill_intr_reset_to_node_if_there(prepSendNode);
        fill_hold_to_node_if_there    (prepSendNode);
        /** assign assignment node*/
        Operable* readyFinal = nullptr;
        readyFinal = _acceptCond;
        readyFinal = addLogicWithOutput(&_syncMeta._syncSlaveReady, readyFinal, BITWISE_AND);

        assert(readyFinal != nullptr);
        Operable* notReadyFinal = &(~(*readyFinal));

        prepSendNode->add_depend_node(prepSendNode, notReadyFinal);
            /** add slave assignment node*/
        for (auto nd : _basic_nodes){
            assert(nd->get_node_type() == ASM_NODE);
            prepSendNode->add_slave_asm_node((AsmNode*)nd, readyFinal);
        }

        /** exit Node*/
        exitNode->add_depend_node(prepSendNode, readyFinal);
        /** assign node*/
        exitNode->assign();
        /** add system node*/
        add_sys_node(prepSendNode);
        add_sys_node(exitNode);
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
        on_attach_block();
    }

    void FlowBlockZyncBase::doPostFunction(){
        on_detach_block();
    }

}
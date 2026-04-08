//
// Created by tanawin on 1/3/2025.
//

#include "pipe.h"

#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockPipeBase::FlowBlockPipeBase(SyncMeta& syncMeta):
    FlowBlockBase(PIPE_BLOCK,
        {
            {FLOW_ST_BASE_STACK, FLOW_ST_PIP_BLK},
            FLOW_JO_SUB_FLOW,
            true
        }),
    _syncMata(syncMeta),
    _pipeName("Pipe_"+ syncMeta.getName()){
    }


    FlowBlockPipeBase::~FlowBlockPipeBase(){
        delete entNode;
        delete waitNode;
        delete exitDummy;
        delete resultNodeWrap;
    }


    ///////////// ELEMENT ADDING
    void FlowBlockPipeBase::add_basic_node(Node* node){
        assert(false);
        // because the implicit par block must be assign to this system
    }

    void FlowBlockPipeBase::add_sub_flow_block(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    void FlowBlockPipeBase::add_con_flow_block(FlowBlockBase* conBlock){
        ////// it can't have con block for pipeline block
        assert(false);
    }

    void FlowBlockPipeBase::add_abandon_flow_block(FlowBlockBase* abandonBlock){
        ////// it can't have abandon block for pipeline block
        assert(false);
    }

    NodeWrap* FlowBlockPipeBase::sumarize_block(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockPipeBase::assignReadySignal(){
        //////// wait signal and last stage means that it is ready
        ////(*_syncMata._syncSlaveReady) = (*entNode->getExitOpr());
        _syncMata.setSlaveReady(*entNode->get_exit_opr_ptr());

        //////// set when pipeline is success
        Node* exitNode = subBlockNodeWrap->get_exit_node();
        _syncMata.setSlaveFinish(*exitNode->get_exit_opr_ptr());
    }

    void FlowBlockPipeBase::build_hw_master(){
        ////// fill retrieve slave hold signal
        add_hold_signal(&_syncMata.holdSlaveSignal);
        add_intr_signal(INT_RESET, &_syncMata.killSlaveSignal);
        add_intr_signal(INT_START, &_syncMata.startSlaveSignal);

        ////// use the base  build function
        FlowBlockBase::build_hw_master();
    }


    void FlowBlockPipeBase::build_hw_component(){
        ////// try to find the activate signal
        Operable* activateSignal = nullptr;
        ////// if it auto, we have to  build the auto trigger for the other side signal
        if (isAutoActivatePipe()){ /////// no zync source
            _syncMata.setMasterReady(makeOprVal("pipe_auto_act_" + _pipeName, 1, 1));
            //(*_syncMata._syncMasterReady) = makeOprVal("pipe_auto_act_" + _pipeName, 1, 1);
        }
        activateSignal = &_syncMata._syncMasterReady;

        ////////////// do integritry check
        assert(_con_blocks.empty());
        assert(_sub_blocks.size() == 1);
        subBlockNodeWrap = _sub_blocks[0]->sumarize_block();
        assert(subBlockNodeWrap != nullptr);

        //////////// init all nodes and condition

        entNode       = new PseudoNode(1, BITWISE_OR);
        waitNode      = new StateNode(get_clock_mode());
        exitDummy     = new DummyNode(&makeOprVal("exitDummy",1, 0));

        ///////// add node dependency
        entNode->add_depend_node(subBlockNodeWrap->get_exit_node(), nullptr);
        entNode->add_depend_node(waitNode, nullptr);
        fill_intr_reset_to_node_if_there(waitNode);
        fill_hold_to_node_if_there(waitNode);
        if(is_there_intr_start()){
            entNode->add_depend_node(_int_nodes[INT_START], nullptr);
        }
        waitNode->add_depend_node(entNode, &(~(*activateSignal)));
        subBlockNodeWrap->add_depend_node_to_all_node(entNode, activateSignal);

        ////////// add system Node
        add_sys_node(entNode);
        add_sys_node(waitNode);
        add_sys_node(exitDummy);

        ////////// assign Node
                ////// ent wait for outer  block assign or master
        waitNode->assign();
        exitDummy->assign();
        subBlockNodeWrap->assign_all_node();

        /////////// build resultNode Wrap
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->add_entrace_node(entNode);
        resultNodeWrap->add_exit_node(exitDummy);

        //////////// build ready signal to tell that pipe line is ready
        assignReadySignal();

    }

    void FlowBlockPipeBase::add_md_log(MdLogVal* mdLogVal){
        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        mdLogVal->addVal( "entNode " +       entNode->get_md_ident_val()      + " " + entNode->get_md_describe());
        mdLogVal->addVal( "waitNode " +      waitNode->get_md_ident_val()     + " " + waitNode->get_md_describe());
        mdLogVal->addVal( "exitDummy " +     exitDummy->get_md_ident_val()    + " " + exitDummy->get_md_describe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrap->get_md_ident_val() + " " + resultNodeWrap->get_md_describe());

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->add_md_log(subLog);

    }

    ///////////// FLOW BLOCK

    void FlowBlockPipeBase::on_attach_block(){
        _ctrl->on_attach_flowBlock(this);
        /*** in pipe we implecitcally add parallel sub Block to system*/
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->on_attach_block();
    }

    void FlowBlockPipeBase::on_detach_block(){
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->on_detach_block();
        assert(isGetFlowBlockYet);
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeBase::do_pre_function(){
        on_attach_block();
    }

    void FlowBlockPipeBase::do_post_function(){
        on_detach_block();
    }

}

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
        createReadySignal();
    }


    FlowBlockPipeBase::~FlowBlockPipeBase(){
        delete entNode;
        delete waitNode;
        delete exitDummy;
        delete resultNodeWrap;
    }


    ///////////// ELEMENT ADDING
    void FlowBlockPipeBase::addElementInFlowBlock(Node* node){
        assert(false);
        // because the implicit par block must be assign to this system
    }

    void FlowBlockPipeBase::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    void FlowBlockPipeBase::addConFlowBlock(FlowBlockBase* conBlock){
        ////// it can't have con block for pipeline block
        assert(false);
    }

    void FlowBlockPipeBase::addAbandonFlowBlock(FlowBlockBase* abandonBlock){
        ////// it can't have abandon block for pipeline block
        assert(false);
    }

    NodeWrap* FlowBlockPipeBase::sumarizeBlock(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockPipeBase::createReadySignal(){
        pipeReadySig = new expression(1);
        ////// we can't directly set data to the _syncSlaveReady because it is operable
        _syncMata._syncSlaveReady = pipeReadySig;
    }

    void FlowBlockPipeBase::assignReadySignal(){
        //////// wait signal and last stage means that it is ready
        (*pipeReadySig) = (*entNode->getExitOpr());
    }

    void FlowBlockPipeBase::buildHwMaster(){
        ////// fill retrieve slave sync signal
        for (Operable* holdOpr: _syncMata.slaveHoldSignals){
            assert(holdOpr != nullptr);
            addHoldSignal(holdOpr);
        }
        ////// use the base  build function
        FlowBlockBase::buildHwMaster();
    }


    void FlowBlockPipeBase::buildHwComponent(){
        ////// try to find the activate signal
        Operable* activateSignal = nullptr;
        if (isAutoActivatePipe()){
            activateSignal = &makeOprVal("pipe_auto_act_" + _pipeName, 1, 1);
        }else{
            _syncMata.tryBuildmatchSignal();
            activateSignal = _syncMata._syncMatched;
            mfAssert(activateSignal != nullptr, "there is no one trigger the pipeline, " + _pipeName +
                                                "if you want it to auto restart, please use autoActivate autoStart");
        }


        ////////////// do integritry check
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);

        //////////// init all nodes and condition

        entNode       = new PseudoNode(1, BITWISE_OR);
        waitNode      = new StateNode(getClockMode());
        exitDummy     = new DummyNode(&makeOprVal("exitDummy",1, 0));

        ///////// add node dependency
        entNode->addDependNode(subBlockNodeWrap->getExitNode(), nullptr);
        entNode->addDependNode(waitNode, nullptr);
        fillIntResetToNodeIfThere(waitNode);
        fillHoldToNodeIfThere(waitNode);
        if(isThereIntStart()){
            entNode->addDependNode(intNodes[INT_START], nullptr);
        }
        waitNode->addDependNode(entNode, &(~(*activateSignal)));
        subBlockNodeWrap->addDependNodeToAllNode(entNode, activateSignal);

        ////////// add system Node
        addSysNode(entNode);
        addSysNode(waitNode);
        addSysNode(exitDummy);

        ////////// assign Node
                ////// ent wait for outer  block assign or master
        waitNode->assign();
        exitDummy->assign();
        subBlockNodeWrap->assignAllNode();

        /////////// build resultNode Wrap
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(entNode);
        resultNodeWrap->addExitNode(exitDummy);


        //////////// build ready signal to tell that pipe line is ready
        assignReadySignal();

    }

    void FlowBlockPipeBase::addMdLog(MdLogVal* mdLogVal){
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal( "entNode " +       entNode->getMdIdentVal()      + " " + entNode->getMdDescribe());
        mdLogVal->addVal( "waitNode " +      waitNode->getMdIdentVal()     + " " + waitNode->getMdDescribe());
        mdLogVal->addVal( "exitDummy " +     exitDummy->getMdIdentVal()    + " " + exitDummy->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrap->getMdIdentVal() + " " + resultNodeWrap->getMdDescribe());

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->addMdLog(subLog);

    }

    ///////////// FLOW BLOCK

    void FlowBlockPipeBase::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
        /*** in pipe we implecitcally add parallel sub Block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockPipeBase::onDetachBlock(){
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeBase::doPreFunction(){
        onAttachBlock();
    }

    void FlowBlockPipeBase::doPostFunction(){
        onDetachBlock();
    }

}

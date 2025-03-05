//
// Created by tanawin on 1/3/2025.
//

#include "pipe.h"

#include "model/controller/controller.h"
#include "pipePooler.h"


namespace kathryn{


    FlowBlockPipeBase::FlowBlockPipeBase(const std::string& pipeName):
    FlowBlockBase(PIPE_BLOCK,
        {
            {FLOW_ST_BASE_STACK},
            FLOW_JO_SUB_FLOW,
            true
        }),
    _pipeName(pipeName){
        PipePooler* pipePooler = getPipePooler();
        pipePooler->addPipeBlk(this);
        readySignal = new expression(1);
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

    Operable* FlowBlockPipeBase::createActivateCond(){

        if (isAutoActivatePipe()){
            return &makeOprVal("autoStartPipe_" + _pipeName, 1,1);
        }
        ////// get tran signaling
        PipePooler* pipePooler = getPipePooler();
        assert(pipePooler != nullptr);
        Operable* tranReadySignal = pipePooler->getTranReadySignal(getPipeName());

        return tranReadySignal;
    }

    void FlowBlockPipeBase::buildHwComponent(){

        Operable* activateSignal = createActivateCond();
        ////////////// do integritry check
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);

        //////////// init all nodes and condition

        entNode       = new PseudoNode(1, BITWISE_OR);
        waitNode      = new StateNode();
        exitDummy     = new DummyNode(&makeOprVal("exitDummy",1, 0));

        ///////// add node dependency
        entNode->addDependNode(subBlockNodeWrap->getExitNode(), nullptr);
        entNode->addDependNode(waitNode, nullptr);
        fillIntResetToNodeIfThere(waitNode);
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


        //////////// build ready signal
        buildReadySignal();

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

    void FlowBlockPipeBase::buildReadySignal(){
        //////// wait signal and last stage means that it is ready
        (*readySignal) = (*entNode->getExitOpr());

    }

}

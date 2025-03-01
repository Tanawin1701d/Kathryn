//
// Created by tanawin on 1/3/2025.
//

#include "pipe.h"

#include <model/controller/controller.h>


namespace kathryn{


    FlowBlockPipe::FlowBlockPipe(const std::string& pipName):
    FlowBlockBase(PIPE_BLOCK,
        {
            {FLOW_ST_BASE_STACK},
            FLOW_JO_SUB_FLOW,
            true
        }),
    _pipeName(pipName)
    {}


    FlowBlockPipe::~FlowBlockPipe(){
        delete entNode;
        delete waitNode;
        delete pipeStartNode;
        delete exitDummy;
    }


    ///////////// ELEMENT ADDING
    void FlowBlockPipe::addElementInFlowBlock(Node* node){
        assert(false);
        // because the implicit par block must be assign to this system
    }

    void FlowBlockPipe::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    void FlowBlockPipe::addConFlowBlock(FlowBlockBase* conBlock){
        ////// it can't have con block for pipeline block
        assert(false);
    }

    void FlowBlockPipe::addAbandonFlowBlock(FlowBlockBase* abandonBlock){
        ////// it can't have abandon block for pipeline block
        assert(false);
    }

    NodeWrap* FlowBlockPipe::sumarizeBlock(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockPipe::createActivateCond(){
        if (isAutoActivatePipe()){
            pipeActivateCond = &makeOprVal("exitDummy",1, 1);
        }else{
            /////// TODO get from instruction pooler
        }
    }


    void FlowBlockPipe::buildHwComponent(){
        //////// TODO pipe activate cond must be assigned
        createActivateCond();
        ////////////// do integritry check
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);

        //////////// init all nodes and condition

        entNode       = new PseudoNode(1, BITWISE_OR);
        waitNode      = new StateNode();
        pipeStartNode = new PseudoNode(1, BITWISE_OR);
        exitDummy     = new DummyNode(&makeOprVal("exitDummy",1, 0));

        ///////// add node dependency
        entNode->addDependNode(subBlockNodeWrap->getExitNode(), nullptr);
        entNode->addDependNode(waitNode, nullptr);
        if(isThereIntStart()){
            entNode->addDependNode(intNodes[INT_START], nullptr);
        }
        waitNode->addDependNode(entNode, &(~(*pipeActivateCond)));
        pipeStartNode->addDependNode(waitNode, pipeActivateCond);
        pipeStartNode->addDependNode(entNode , pipeActivateCond);
        subBlockNodeWrap->addDependNodeToAllNode(pipeStartNode, nullptr);

        ////////// add system Node
        addSysNode(entNode);
        addSysNode(waitNode);
        addSysNode(pipeStartNode);
        addSysNode(exitDummy);

        ////////// assign Node
                ////// ent wait for outer  block assign or master
        waitNode->assign();
        pipeStartNode->assign();
        exitDummy->assign();
        subBlockNodeWrap->assignAllNode();

        /////////// build resultNode Wrap
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(entNode);
        resultNodeWrap->addExitNode(exitDummy);


    }



    ///////////// FLOW BLOCK

    void FlowBlockPipe::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
        /*** in pipe we implecitcally add parallel sub Block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockPipe::onDetachBlock(){
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }




    void FlowBlockPipe::doPreFunction(){
        onAttachBlock();
    }

    void FlowBlockPipe::doPostFunction(){
        onDetachBlock();
    }



}

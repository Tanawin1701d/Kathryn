//
// Created by tanawin on 7/8/2024.
//

#include "pick.h"

#include <model/controller/controller.h>


namespace kathryn{

    FlowBlockPick::FlowBlockPick():
       FlowBlockBase(PICK,
         {
                 {FLOW_ST_BASE_STACK},
                 FLOW_JO_SUB_FLOW,
                 true
         })
    {}

    FlowBlockPick::~FlowBlockPick(){
        delete resultNodeWrap;
        delete jointNode;
        delete autoExitNode;
        delete exitNode;
    }

    void FlowBlockPick::addElementInFlowBlock(Node* node){
        assert(false);
    }

    void FlowBlockPick::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(subBlock->getFlowType() == PICK_WHEN);
        FlowBlockBase::addSubFlowBlock(subBlock);
        pickCondBlocks.push_back(dynamic_cast<FlowBlockPickCond*>(subBlock));
    }

    void FlowBlockPick::addConFlowBlock(FlowBlockBase* conBlock){
        assert(false);
    }



    NodeWrap* FlowBlockPick::sumarizeBlock(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockPick::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPick::onDetachBlock(){
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPick::buildHwComponent(){
        ////// summarize all block
        for (auto pickCondBlock: pickCondBlocks){
            nodeWrapOfPickCondBlocks.push_back(pickCondBlock->sumarizeBlock());
        }
        assert(!nodeWrapOfPickCondBlocks.empty());
        assert(!pickCondBlocks.empty());
        assert(nodeWrapOfPickCondBlocks.size() == pickCondBlocks.size());
        assert(_conBlocks.empty());

        /////// build start node
        jointNode = new PseudoNode(1, BITWISE_OR);
        jointNode->setInternalIdent("jointOfPickNode" + std::to_string(getGlobalId()));
        if (isThereIntStart()){
            jointNode->addDependNode(intNodes[INT_START], nullptr);
        }
        addSysNode(jointNode);
        for (int sid = 0; sid < nodeWrapOfPickCondBlocks.size(); sid++){
            nodeWrapOfPickCondBlocks[sid]
            ->addDependNodeToAllNode(jointNode,pickCondBlocks[sid]->getCondition());
            nodeWrapOfPickCondBlocks[sid]->assignAllNode();
        }

        ////// build auto exitNode if needed
        if (reqAutoExit){
            autoExitNode = new PseudoNode(1, BITWISE_AND);
            addSysNode(autoExitNode);
            autoExitNode->setInternalIdent("pickAutoExit" + std::to_string(getGlobalId()));
            Operable* allFalse = nullptr;
            for (FlowBlockPickCond* fpc: pickCondBlocks){
                assert(fpc != nullptr);
                if (allFalse == nullptr){
                    allFalse = &(!*fpc->getCondition());
                }else{
                    allFalse = &((*allFalse) & (!*fpc->getCondition()));
                }
            }
            assert(allFalse != nullptr);
            autoExitNode->addDependNode(jointNode, allFalse);
            autoExitNode->assign();
        }

        /////// build exit node
        exitNode = new PseudoNode(1, BITWISE_OR);
        addSysNode(exitNode);
        exitNode->setInternalIdent("pickExit" + std::to_string(getGlobalId()));
        ////// join all exit node
        for (auto & nodeWrapOfPickCondBlock : nodeWrapOfPickCondBlocks){
            exitNode->addDependNode(
                nodeWrapOfPickCondBlock->getExitNode(),
                nullptr);
        }
        if (autoExitNode != nullptr){
            exitNode->addDependNode(autoExitNode, nullptr);
        }

        exitNode->assign();

        ///////
        ///
        ///result node warp
        ///
        ///////

        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(jointNode);
        resultNodeWrap->addExitNode(exitNode);

        //// do force exit node
        genSumForceExitNode(nodeWrapOfPickCondBlocks);
        if (_areThereForceExit){
            resultNodeWrap->addForceExitNode(_forceExitNode);
        }

        //// cycle determiner
        NodeWrapCycleDet deter;
        deter.addToDet(nodeWrapOfPickCondBlocks);
        if (reqAutoExit){
            deter.addToDet(0);
        }
        int cycleUsed = deter.getSameCycleHorizon();
        resultNodeWrap->setCycleUsed(cycleUsed);
    }

    void FlowBlockPick::doPreFunction() {
        onAttachBlock();
    }

    void
    FlowBlockPick::doPostFunction(){
        onDetachBlock();
    }











}

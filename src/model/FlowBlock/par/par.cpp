//
// Created by tanawin on 4/12/2566.
//

#include "par.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockPar::FlowBlockPar(): FlowBlockBase(PARALLEL) {}

    NodeWrap*
    FlowBlockPar::sumarizeBlock() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }


    void
    FlowBlockPar::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
    }

    void
    FlowBlockPar::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void
    FlowBlockPar::buildHwComponent() {
        assert((!basicNodes.empty()) || (!subBlocks.empty()));
        /**build result node wrap*/
        resultNodeWrap = new NodeWrap();
        /** build node for basicNode*/
        if (!basicNodes.empty()){
            basicStNode = new StateNode();
            for (auto nd : basicNodes){
                nd->addDependState(basicStNode, BITWISE_AND);
                nd->assign();
            }
        }
        /**build node wrap for flowblock*/
        for (auto fb : subBlocks){
            NodeWrap* nw = fb->sumarizeBlock();
            assert(nw != nullptr);
            nodeWrapOfSubBlock.push_back(nw);
        }

        /**determine cycle used*/
        NodeWrapCycleDet cycleDet;
        if (basicStNode != nullptr)
            cycleDet.addToDet(basicStNode);
        cycleDet.addToDet(nodeWrapOfSubBlock);
        int cycleUsed = cycleDet.getMaxCycleHorizon();

        /** build syn node if need*/
        /////// syn reg needed
        int synSize = (basicStNode != nullptr) + (int)nodeWrapOfSubBlock.size();
        assert(synSize > 0);
        synNode = new SynNode(synSize);
        if (basicStNode != nullptr){
            synNode->addDependState(basicStNode, BITWISE_AND);
        }
        for (auto nw : nodeWrapOfSubBlock){
            synNode->addDependState(nw, BITWISE_AND);
        }
        ////// assign sync reg
        synNode->assign();

        /*** entrance node management*/
        resultNodeWrap->addEntraceNode(basicStNode);
        for (auto nw : nodeWrapOfSubBlock){
            resultNodeWrap->transferNodeFrom(nw);
        }
        /*** exit node*/
        resultNodeWrap->addExitNode(synNode);


    }

    void
    FlowBlockPar::doPreFunction() {
        onAttachBlock();
    }

    void
    FlowBlockPar::doPostFunction(){
        onDetachBlock();
    }

    /**
     *
     *
     * parallel block auto
     *
     *
     * */
    void
    FLowBlockParAuto::buildHwComponent() {
        assert((!basicNodes.empty()) || (!subBlocks.empty()));
        /**build result node wrap*/
        resultNodeWrap = new NodeWrap();
        /** build node for basicNode*/
        if (!basicNodes.empty()){
            basicStNode = new StateNode();
            for (auto nd : basicNodes){
                nd->addDependState(basicStNode, BITWISE_AND);
                nd->assign();
            }
            basicStNode->assign();
        }
        /**build node wrap for flowblock*/
        for (auto fb : subBlocks){
            NodeWrap* nw = fb->sumarizeBlock();
            assert(nw != nullptr);
            nodeWrapOfSubBlock.push_back(nw);
        }

        /**determine cycle used*/
        NodeWrapCycleDet cycleDet;
        if (basicStNode != nullptr)
            cycleDet.addToDet(basicStNode);
        cycleDet.addToDet(nodeWrapOfSubBlock);
        int cycleUsed = cycleDet.getMaxCycleHorizon();

        /** build syn node if need*/
        if ( cycleUsed == IN_CONSIST_CYCLE_USED ){
            /////// syn reg needed
            int synSize = (basicStNode != nullptr) + (int)nodeWrapOfSubBlock.size();
            assert(synSize > 0);
            synNode = new SynNode(synSize);
            if (basicStNode != nullptr){
                synNode->addDependState(basicStNode, BITWISE_AND);
            }
            for (auto nw : nodeWrapOfSubBlock){
                synNode->addDependState(nw, BITWISE_AND);
            }
            ////// assign sync reg
            synNode->assign();
        }

        /** set result node wrap set */

        /*** entrance node management*/
        resultNodeWrap->addEntraceNode(basicStNode);
        for (auto nw : nodeWrapOfSubBlock){
            resultNodeWrap->transferNodeFrom(nw);
        }
        /*** exit node*/
        if (synNode != nullptr){
            resultNodeWrap->addExitNode(synNode);
        }else{
            /** get Match allow nullptr*/
            assert(cycleUsed > 0);
            Node* exitNode = getMatchExitExpr({basicStNode},
                                                 nodeWrapOfSubBlock,
                                                 cycleUsed
                                                 );
            assert(exitNode != nullptr);
            resultNodeWrap->addExitNode(exitNode);
        }

    }

}

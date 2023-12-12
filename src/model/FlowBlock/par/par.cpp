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

        /** build sub component*/
//        for (auto sb : subBlocks){
//            sb->buildHwComponent();
//        }

        /**build result node wrap*/
        resultNodeWrap = new NodeWrap();

        /** build state register for basic node if needed*/
        if (!basicNodes.empty()){
            basicStReg = new StateReg();
            /** for basic assignment */
            for (auto nd: basicNodes){
                nd->addDependState(basicStReg, BITWISE_AND);
                nd->assign();
            }
            /** for result Node Wrap*/
            basicStNode = basicStReg->generateStateNode();
            resultNodeWrap->addEntraceNode(basicStNode);
        }


        /** get node warp */

        for (auto sb: subBlocks){
            nodeWrapOfSubBlock.push_back(sb->sumarizeBlock());
        }

        /** add sub node to entrance to result*/
        for (auto nw: nodeWrapOfSubBlock){
            assert(nw != nullptr);
            resultNodeWrap->transferNodeFrom(nw);
        }

        if (!subBlocks.empty()){
            int assignNo = 0;
            auto syncReg = new StateReg((int)subBlocks.size() + (!basicNodes.empty()));
            //// assign state for sync register from basic State
            if (!basicNodes.empty()){
                assert(basicStReg != nullptr);
                syncReg->addUpdateEvent(basicStReg, assignNo);
            }

            for (auto nw: nodeWrapOfSubBlock){
                //// assign state for sync register from complex subblock
                syncReg->addUpdateEvent(nw->exitOpr, assignNo++);
            }
            //// assign sync register to Nodewarp
            resultNodeWrap->addExitOpr(syncReg->generateEndExpr());
        }else{
            resultNodeWrap->addExitOpr(basicStReg->generateEndExpr());
        }

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

        /** build state register for basic node if needed*/
        if (!basicNodes.empty()){
            basicStReg = new StateReg();
            /** for basic assignment */
            for (auto nd: basicNodes){
                nd->addDependState(basicStReg, BITWISE_AND);
                nd->assign();
            }
            /** for result Node Wrap*/
            basicStNode = basicStReg->generateStateNode();
            resultNodeWrap->addEntraceNode(basicStNode);
        }


        /** get node warp */

        for (auto sb: subBlocks){
            nodeWrapOfSubBlock.push_back(sb->sumarizeBlock());
        }

        /** add sub node to entrance to result*/
        for (auto nw: nodeWrapOfSubBlock){
            assert(nw != nullptr);
            resultNodeWrap->transferNodeFrom(nw);
        }

        if (!subBlocks.empty()){

            /** determine used cycle*/
            NodeWrapCycleDet cycleDet;
            cycleDet.addToDet(nodeWrapOfSubBlock);
            if (!basicNodes.empty()) {
                cycleDet.addToDet(basicStNode);
            }
            int cycleUsed = cycleDet.getMaxCycleHorizon();

            if (cycleUsed == NodeWrap::IN_CONSIST_CYCLE_USED){
                int assignNo = 0;
                auto syncReg = new StateReg((int)subBlocks.size() + (!basicNodes.empty()));
                //// assign state for sync register from basic State
                if (!basicNodes.empty()){
                    assert(basicStReg != nullptr);
                    syncReg->addUpdateEvent(basicStReg, assignNo++);
                }
                for (auto nw: nodeWrapOfSubBlock){
                    //// assign state for sync register from complex subblock
                    syncReg->addUpdateEvent(nw->exitOpr, assignNo++);
                }
                resultNodeWrap->addExitOpr(syncReg->generateEndExpr());
            }else{
                /** cycle used is consistent*/
                Operable* exitOpr = nullptr;
                /** in case it is state node*/
                if (basicStNode != nullptr){
                    Node* matchNode = cycleDet.getMatchNode({basicStNode}, cycleUsed);
                    if (matchNode == basicStNode){
                        exitOpr = basicStReg->generateEndExpr();
                    }
                }
                /** in case it is flow block*/
                if (exitOpr == nullptr){
                    NodeWrap* matchNw = cycleDet.getMatchNodeWrap(
                            nodeWrapOfSubBlock, cycleUsed);
                    if (matchNw != nullptr){
                        exitOpr = matchNw->exitOpr;
                    }
                }
                assert(exitOpr != nullptr);
                resultNodeWrap->setCycleUsed(cycleUsed);
                resultNodeWrap->addExitOpr(exitOpr);
            }

            //// assign sync register to Nodewarp

        }else{
            assert(!basicNodes.empty());
            resultNodeWrap->setCycleUsed(1);
            resultNodeWrap->addExitOpr(basicStReg->generateEndExpr());
        }

    }

}

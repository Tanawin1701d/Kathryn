//
// Created by tanawin on 4/12/2566.
//

#include "par.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockPar::FlowBlockPar(FLOW_BLOCK_TYPE fbType): FlowBlockBase(fbType) {
        assert((fbType == PARALLEL_AUTO_SYNC) ||
               (fbType == PARALLEL_NO_SYN));
    }

    FlowBlockPar::~FlowBlockPar(){
        delete resultNodeWrap;
        delete basicStNode;
        delete synNode;
        delete pseudoExitNode;
        FlowBlockBase::~FlowBlockBase();
    }

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

        /** build node for basic assignment*/
        if (!basicNodes.empty()){
            basicStNode = new StateNode();
            /** add basic assignment to depend on stateNode*/
            for (auto nd : basicNodes){
                nd->addDependNode(basicStNode);
                nd->setDependStateJoinOp(BITWISE_AND);
                nd->assign();
            }
        }
        /**build node wrap for flowblock and keepTrack that node have forceExitOpr*/
        for (auto fb : subBlocks){
            NodeWrap* nw = fb->sumarizeBlock();
            assert(nw != nullptr);
            nodeWrapOfSubBlock.push_back(nw);
        }
        /** force exit node */
        genSumForceExitNode(nodeWrapOfSubBlock);

        /**
         *
         * determine basic cycle used
         *
         * */
        NodeWrapCycleDet cycleDet;
        if (basicStNode != nullptr)
            cycleDet.addToDet(basicStNode);
        cycleDet.addToDet(nodeWrapOfSubBlock);
        cycleUsed = cycleDet.getMaxCycleHorizon();
        /**
         *
         * build result node wrap entrance
         * */
        /*** entrance node management*/
        resultNodeWrap = new NodeWrap();
        if (basicStNode != nullptr)
            resultNodeWrap->addEntraceNode(basicStNode);
        for (auto nw : nodeWrapOfSubBlock){
            resultNodeWrap->transferEntNodeFrom(nw);
        }

        buildSyncNode();
        assignExitToRnw();
        assignCycleUsedToRnw();
        assignForceExitToRnw();
    }


    void FlowBlockPar::assignCycleUsedToRnw(){
        resultNodeWrap->setCycleUsed(cycleUsed);
    }

    void FlowBlockPar::assignForceExitToRnw() {
        if (areThereForceExit){
            resultNodeWrap->addForceExitNode(forceExitNode);
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

    std::string FlowBlockPar::getMdDescribe() {
        std::string ret;

        ret += "basicStateNode is " +
                ((basicStNode != nullptr) ?
                basicStNode->getMdIdentVal() + "  " + basicStNode->getMdDescribe():
                ""
                ) +"\n";

        ret += "synNode is " +
                ((synNode != nullptr) ?
                synNode->getMdIdentVal() + "  " + synNode->getMdDescribe():
                "") + "\n";

        ret += "pseudoExitNode is " +
                ((pseudoExitNode != nullptr) ?
                pseudoExitNode->getMdIdentVal() + "  " + pseudoExitNode->getMdDescribe():
                "") + "\n";

        ret += getMdDescribeRecur();
        ret += "\n";

        return ret;
    }

    void FlowBlockPar::addMdLog(MdLogVal *mdLogVal) {


        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");

        mdLogVal->addVal("basicStateNode is " +
                         ((basicStNode != nullptr) ?
                          basicStNode->getMdIdentVal() + "  " + basicStNode->getMdDescribe():
                          ""
                         ));

        mdLogVal->addVal("synNode is " +
                         ((synNode != nullptr) ?
                          synNode->getMdIdentVal() + "  " + synNode->getMdDescribe():
                          ""));

        mdLogVal->addVal("pseudoExitNode is " +
                         ((pseudoExitNode != nullptr) ?
                          pseudoExitNode->getMdIdentVal() + "  " + pseudoExitNode->getMdDescribe():
                          ""));

        addMdLogRecur(mdLogVal);
    }


    /**
     *
     *
     * parallel block auto
     *
     *
     * */

    void FlowBlockParAuto::buildSyncNode() {
        int amt_block = ((basicStNode != nullptr) ? 1 : 0) +
                        (int)(nodeWrapOfSubBlock.size());
        /** build syn node if need*/
        if ( (cycleUsed == IN_CONSIST_CYCLE_USED) &&
             (amt_block > 1) /** incase amt_block == 1 we don't have to sync*/
                ){
            /////// syn reg needed
            int synSize = amt_block;
            synNode = new SynNode(synSize);
            /**syn node don't need to specify join operation due to it used own logic or*/
            if (basicStNode != nullptr){
                synNode->addDependNode(basicStNode);
            }
            for (auto nw : nodeWrapOfSubBlock){
                synNode->addDependNode(nw->getExitNode());
            }
            ////// assign sync reg and sync node don't have to set join op because
            /////////// sync register will handle it
            synNode->assign();
        }

    }

    void FlowBlockParAuto::assignExitToRnw() {

        int amt_block = ((basicStNode != nullptr) ? 1 : 0) +
                        (int)(nodeWrapOfSubBlock.size());

        if (synNode != nullptr){
            resultNodeWrap->addExitNode(synNode);
        }else{
            /** get Match allow nullptr*/
            Node* exitNode = nullptr;
            if (cycleUsed >= 0){
                exitNode = getMatchNodeFromNdsOrNws({basicStNode},
                                                    nodeWrapOfSubBlock,
                                                    cycleUsed);
            }else{
                assert(amt_block == 1);
                exitNode = getAnyNodeFromNdsOrNws({basicStNode},
                                                  nodeWrapOfSubBlock);
            }
            assert(exitNode != nullptr);
            resultNodeWrap->addExitNode(exitNode);
        }

    }


    /**
     *
     *
     * parallel block no sync
     *
     *
     * */
    void FlowBlockParNoSync::assignExitToRnw() {

        int amt_block = ((basicStNode != nullptr) ? 1 : 0) +
                        (int)(nodeWrapOfSubBlock.size());

        /** get Match allow nullptr*/
        Node* exitNode = nullptr;
        if (cycleUsed >= 0){
            exitNode = getMatchNodeFromNdsOrNws({basicStNode},
                                                nodeWrapOfSubBlock,
                                                cycleUsed);
        }else if (amt_block == 1){
            assert(amt_block == 1);
            exitNode = getAnyNodeFromNdsOrNws({basicStNode},
                                              nodeWrapOfSubBlock);
        }else{
            assert(amt_block > 1);
            pseudoExitNode = new PseudoNode();
            if (basicStNode != nullptr)
                pseudoExitNode->addDependNode(basicStNode);
            for (auto nw : nodeWrapOfSubBlock){
                pseudoExitNode->addDependNode(nw->getExitNode());
            }
            pseudoExitNode->setDependStateJoinOp(BITWISE_OR);
            pseudoExitNode->assign();
            exitNode  = pseudoExitNode;
        }
        assert(exitNode != nullptr);
        resultNodeWrap->addExitNode(exitNode);
    }
}

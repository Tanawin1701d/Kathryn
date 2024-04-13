//
// Created by tanawin on 4/12/2566.
//

#include "par.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockPar::FlowBlockPar(FLOW_BLOCK_TYPE fbType):
    FlowBlockBase(fbType,
      {
              {FLOW_ST_BASE_STACK,
                             FLOW_ST_PATTERN_STACK},
              FLOW_JO_SUB_FLOW,
              true
      })
      {
        assert((fbType == PARALLEL_AUTO_SYNC) ||
               (fbType == PARALLEL_NO_SYN));
    }

    FlowBlockPar::~FlowBlockPar(){
        delete resultNodeWrap;
        delete basicStNode;
        delete synNode;
        delete pseudoExitNode;
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
        mfAssert((!_basicNodes.empty()) || (!_subBlocks.empty()),
                 "parBlock has no assignment"
                 );
        assert(_conBlocks.empty());

        /** build node for basic assignment*/
        if (!_basicNodes.empty()){
            basicStNode = new StateNode();
            basicStNode->setDependStateJoinOp(BITWISE_AND);
            fillResetIntEventToNode(basicStNode);
            /** add basic assignment to depend on stateNode*/
            for (auto nd : _basicNodes){
                assert(nd->getNodeType() == ASM_NODE);
                basicStNode->addSlaveAsmNode((AsmNode*)nd);
            }
        }
        /**build node wrap for flowblock and keepTrack that node have forceExitOpr*/
        for (auto fb : _subBlocks){
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

        /*** process entrance of result node wrap*/
        if (_interruptNode[INTR_TYPE_START] == nullptr) {
            /*** case there is no interrupt start,
             * simply just provide node to upper next
             * */
            if (basicStNode != nullptr)
                resultNodeWrap->addEntraceNode(basicStNode);
            for (auto nw: nodeWrapOfSubBlock) {
                resultNodeWrap->transferEntNodeFrom(nw);
            }
        }else{
            /** case there is interrupt start*/
            genStartBlockNode();
            resultNodeWrap->addEntraceNode(_upStart);

            if (basicStNode != nullptr){
                basicStNode->addDependNode(_mainStart);
                basicStNode->assign();
            }
            for (auto nw: nodeWrapOfSubBlock) {
                resultNodeWrap->transferEntNodeFrom(nw);
                nw->addDependNodeToAllNode(_mainStart);
                nw->assignAllNode();
            }
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
        if (_areThereForceExit){
            resultNodeWrap->addForceExitNode(_forceExitNode);
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

        Node* exitNode = resultNodeWrap->getExitNode();
        mdLogVal->addVal("exit node is " +
                        ( (exitNode != nullptr) ?
                            exitNode->getMdIdentVal() + "  " + exitNode->getMdDescribe():
                            ""));

        if (resultNodeWrap->isThereForceExitNode()){
            mdLogVal->addVal("forceExit is " + resultNodeWrap->getForceExitNode()->getMdIdentVal() +
                             "  " +
                             resultNodeWrap->getForceExitNode()->getMdDescribe());
        }

        addMdLogRecur(mdLogVal);
    }

    void FlowBlockPar::simStartCurCycle() {

        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isStateRunning = false;
        /** simulate each element*/
        for (auto _sb: _subBlocks){
            assert(_sb != nullptr);
            _sb->simStartCurCycle();
            isStateRunning |= _sb->isBlockOrNodeRunning();
        }
        for (auto _bsAsmNd: _basicNodes){
            assert(_bsAsmNd != nullptr);
            _bsAsmNd->simStartCurCycle();
        }
        if (basicStNode != nullptr){
            basicStNode->simStartCurCycle();
            isStateRunning |= basicStNode->isBlockOrNodeRunning();
        }
//        if (synNode != nullptr){
//            /**syn node is not necessary anymore*/
//            synNode->simStartCurCycle();
//            isStateRunning |= synNode->isBlockOrNodeRunning();
//        }
        if (pseudoExitNode != nullptr){
            pseudoExitNode->simStartCurCycle();
            isStateRunning |= pseudoExitNode->isBlockOrNodeRunning();
        }
        /** increment log*/
        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }

    }

    void FlowBlockPar::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        for (auto _sb: _subBlocks){
            assert(_sb != nullptr);
            _sb->simExitCurCycle();
        }
        for (auto _bsAsmNd: _basicNodes){
            assert(_bsAsmNd != nullptr);
            _bsAsmNd->simExitCurCycle();
        }
        if (basicStNode != nullptr){
            basicStNode->simExitCurCycle();
        }
        if (synNode != nullptr){
            synNode->simExitCurCycle();
        }
        if (pseudoExitNode != nullptr){
            pseudoExitNode->simExitCurCycle();
        }
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
            synNode->setInternalIdent(
                    "parSynNode_" +
                    std::to_string(getGlobalId())
                    );
            /**syn node don't need to specify join operation due to it used own logic or*/
            if (basicStNode != nullptr){
                synNode->addDependNode(basicStNode);
            }
            for (auto nw : nodeWrapOfSubBlock){
                synNode->addDependNode(nw->getExitNode());
            }
            synNode->setResetIntNode(_interruptNode[INTR_TYPE_RESET]);
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
            pseudoExitNode = new PseudoNode(1);
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

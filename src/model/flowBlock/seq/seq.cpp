//
// Created by tanawin on 5/12/2566.
//

#include "seq.h"
#include "model/controller/controller.h"

namespace kathryn{

    /**
     *
     *
     * sequenceElement
     *
     *
     * */


    SequenceEle::SequenceEle(Node* asmNode) {
        assert(asmNode != nullptr);
        assert(asmNode->getNodeType() == ASM_NODE);
        _asmNode = (AsmNode*)asmNode;
    }

    SequenceEle::SequenceEle(FlowBlockBase* fbBase) {
        assert(fbBase != nullptr);
        _subBlock  = fbBase;
    }

    SequenceEle::~SequenceEle() {
        delete _asmNode;
        delete _stNode;
    }

    void SequenceEle::genNode(){
        /** assign node wrap*/
        if (_asmNode != nullptr){
            _stNode = new StateNode();
            SrcNodeAgent sng = _asmNode->genSrcAgent();
            sng.addDep(_stNode, CON_NODE_SET);
            ////////// node wrap
            _nw.addEntraceNode(sng);
            _nw.setCycleUsed(1);
            _nw.setExitNode(_stNode);
        }else{
            _nw = _subBlock->sumarizeBlock();
        }
    }
    /**
     *
     *
     * sequence flow
     *
     *
     * */

    FlowBlockSeq::FlowBlockSeq(): FlowBlockBase(SEQUENTIAL,
                                                {
                                                        {FLOW_ST_BASE_STACK,
                                                                      FLOW_ST_PATTERN_STACK},
                                                        FLOW_JO_SUB_FLOW,
                                                        true
                                                }),
                                  resultNodeWrap(nullptr) {}

    FlowBlockSeq::~FlowBlockSeq(){
        delete resultNodeWrap;
        for (auto seqEle : _subSeqMetas){
            delete seqEle;
        }
    }

    void FlowBlockSeq::addElementInFlowBlock(Node* node) {
        assert(node != nullptr);
        _subSeqMetas.push_back(new SequenceEle(node));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockSeq::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        _subSeqMetas.emplace_back(new SequenceEle(subBlock));
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockSeq::sumarizeBlock() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockSeq::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockSeq::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockSeq::buildHwComponent() {
        mfAssert(!_subSeqMetas.empty(), "seqBlock has no assignment");
        assert(_conBlocks.empty());
        NodeWrapCycleDet cycleDet;
        /** generate hardware*/
        int idx = 0;
        for (auto& seqMeta: _subSeqMetas) {
            seqMeta->genNode();
            seqMeta->setIdentStateId(getGlobalId(),idx++);
            seqMeta->addToCycleDet(cycleDet);
        }
        /** generate forceExit Node*/
            /***check areThere forceExitNode*/
        std::vector<NodeWrap*> allNw;
        for(auto seqEle: _subSeqMetas){
            if (seqEle->isNodeWrap()){
                allNw.push_back(seqEle->getNodeWrap());
            }
        }
        genSumForceExitNode(allNw);

        /***
         *
         * build dep
         *
         * */

        for (int idx = 1; idx < _subSeqMetas.size(); idx++){

            //////////// assign dep for stateNode
            Operable* ;
            int revIdx = idx-1;
            for ( ; revIdx >= 0; revIdx--){
                if (_subSeqMetas[revIdx]->_nw.getBypassCond() != nullptr){

                }
            }

        }


    }



    std::string FlowBlockSeq::getMdDescribe() {

        std::string ret;
        int eleCnt = 0;
        ret += "[ " + FlowBlockBase::getMdIdentVal() + " ]\n";
        for (auto seqEle: _subSeqMetas){
            ret += seqEle->getDescribe() + "\n";
            eleCnt++;
        }
        ret += getMdDescribeRecur();
        return ret;

    }

    void FlowBlockSeq::addMdLog(MdLogVal *mdLogVal) {

        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        for (auto seqEle: _subSeqMetas){
            mdLogVal->addVal(seqEle->getDescribe());
        }
        if (resultNodeWrap->isThereForceExitNode()){
            mdLogVal->addVal("forceExit is " + resultNodeWrap->getForceExitNode()->getMdIdentVal() +
                                                   "  " +
                                                   resultNodeWrap->getForceExitNode()->getMdDescribe());
        }

        addMdLogRecur(mdLogVal);

    }

    void FlowBlockSeq::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockSeq::doPostFunction() {
        onDetachBlock();
    }

    /** override flow block simulation*/

    void FlowBlockSeq::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isRunning = false;
        /** simulate each element*/
        for (auto subSeqMeta: _subSeqMetas){
            subSeqMeta->simulate();
            isRunning |= subSeqMeta->isBlockOrNodeRunning();
        }
        if (isRunning) {
            setBlockOrNodeRunning();
            incEngine();
        }
    }


    void FlowBlockSeq::simExitCurCycle(){
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        for(auto subSeqMeta: _subSeqMetas){
            subSeqMeta->finalizeSim();
        }
    }



}

//
// Created by tanawin on 5/12/2566.
//

#include "seq.h"
#include "model/controller/controller.h"

namespace kathryn{

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
        _subSeqMetas.pushBack(new SequenceEleBasic(node));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockSeq::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        _subSeqMetas.emplaceBack(new SequenceEleFlowBlock(subBlock));
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
            seqMeta->setIntReset(intNodes[INT_RESET]); //// set interrupt reset must be set before gennode
            seqMeta->setHoldNode(holdNode);
            seqMeta->genNode(getClockMode());
            seqMeta->setIdentStateId(getGlobalId(),idx++);
            seqMeta->addToCycleDet(cycleDet);
            seqMeta->addToSystemNodes(_sysNodes);
        }
        /** generate forceExit Node*/
            /***check areThere forceExitNode*/
        std::vector<NodeWrap*> allNw;
        for(auto seqEle: _subSeqMetas){
            if (seqEle->isNodeWrap()){
                allNw.pushBack(seqEle->getNodeWrap());
            }
        }
        genSumForceExitNode(allNw);
        /** connect depend node chain*/
        for (sizeT idx = 0; (idx+1) < _subSeqMetas.size(); idx++){
            auto lhsNodeWrapper = _subSeqMetas[idx];
            auto rhsNodeWrapper = _subSeqMetas[idx+1];
            rhsNodeWrapper->assignDependDent(lhsNodeWrapper);
        }
        if (isThereIntStart()){
            _subSeqMetas[0]->assignIntStart(intNodes[INT_START]);
        }

        /** build new result NodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNodes((*_subSeqMetas.begin())->getEntranceNodes());
        resultNodeWrap->addExitNode((*_subSeqMetas.rbegin())->getStateFinishIden());
        resultNodeWrap->setCycleUsed(cycleDet.getCycleVertical());
        if (_areThereForceExit)
            resultNodeWrap->addForceExitNode(_forceExitNode);

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
            mdLogVal->addVal("forceExit is " + resultNodeWrap->getForceExitNodePtr()->getMdIdentVal() +
                                                   "  " +
                                                   resultNodeWrap->getForceExitNodePtr()->getMdDescribe());
        }

        addMdLogRecur(mdLogVal);

    }

    void FlowBlockSeq::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockSeq::doPostFunction() {
        onDetachBlock();
    }

}

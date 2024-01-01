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


    SequenceEle::SequenceEle(Node *asmNode) {
        assert(asmNode != nullptr);
        _asmNode = asmNode;
    }

    SequenceEle::SequenceEle(FlowBlockBase *fbBase) {
        assert(fbBase != nullptr);
        _subBlock  = fbBase;
    }

    void SequenceEle::genHardware() {

        assert( (_asmNode != nullptr) ^ (_subBlock != nullptr));

        ///// it is the basic assignment
        if (_asmNode != nullptr){
            _stateNode = new StateNode();
            _asmNode->addDependNode(_stateNode);
            _asmNode->assign();
        }else if (_subBlock != nullptr){
            _complexNode = _subBlock->sumarizeBlock();
        }else{
            assert(false);
        }
    }

    void SequenceEle::addToCycleDet(NodeWrapCycleDet &deter) const {
        if (_asmNode != nullptr){
            deter.addToDet(_asmNode);
        } else if (_subBlock != nullptr){
            deter.addToDet(_complexNode);
        }else{
            assert(true);
        }
    }

    void SequenceEle::assignDependDent(SequenceEle *predecessor) const {
        assert(predecessor != nullptr);
        assert((_asmNode != nullptr) ^ (_subBlock != nullptr));

        if (_asmNode != nullptr){
            _stateNode->addDependNode(predecessor->getStateFinishIden());
            _stateNode->assign();   ///// assign state node to actual value
        }else if (_subBlock != nullptr){
            _complexNode->addDependStateToAllNode(predecessor->getStateFinishIden(), BITWISE_AND);
            _complexNode->assignAllNode();
        }else{
            assert(true);
        }

    }

    Node* SequenceEle::getStateFinishIden() const {
        assert( (_asmNode != nullptr) ^ (_subBlock != nullptr));
        if (_asmNode != nullptr){
            return _stateNode;
        }else if (_subBlock != nullptr){
            return _complexNode->getExitNode();
        }
        assert(true);
        return nullptr;
    }

    std::vector<Node *> SequenceEle::getEntranceNodes() {
        assert( (_asmNode != nullptr) ^ (_subBlock != nullptr));
        if (_asmNode != nullptr){
            return {_stateNode};
        }else if (_subBlock != nullptr){
            return _complexNode->entranceNodes;
        }
        assert(false);
    }

    /**
     *
     *
     * sequence flow
     *
     *
     * */

    FlowBlockSeq::FlowBlockSeq(): FlowBlockBase(SEQUENTIAL),
                                  resultNodeWrap(nullptr) {}

    void FlowBlockSeq::addElementInFlowBlock(Node* node) {
        assert(node != nullptr);
        _subSeqMetas.emplace_back(node);
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockSeq::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        _subSeqMetas.emplace_back(subBlock);
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
        assert(!_subSeqMetas.empty());
        NodeWrapCycleDet cycleDet;
        /** generate hardware*/
        for (auto& seqMeta: _subSeqMetas) {
            seqMeta.genHardware();
            seqMeta.addToCycleDet(cycleDet);
        }
        /** connect  chain*/
        for (size_t idx = 0; (idx+1) < _subSeqMetas.size(); idx++){
            auto& lhsNodeWrapper = _subSeqMetas[idx];
            auto& rhsNodeWrapper = _subSeqMetas[idx+1];
            rhsNodeWrapper.assignDependDent(&lhsNodeWrapper);
        }
        /** build new result NodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNodes(_subSeqMetas.begin()->getEntranceNodes());
        resultNodeWrap->addExitNode(_subSeqMetas.rbegin()->getStateFinishIden());
        resultNodeWrap->setCycleUsed(cycleDet.getCycleVertical());

    }

    void FlowBlockSeq::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockSeq::doPostFunction() {
        onDetachBlock();
    }

}

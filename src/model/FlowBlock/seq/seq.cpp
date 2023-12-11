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


    SequenceEle::SequenceEle(Node *simpleNode) {
        assert(simpleNode != nullptr);
        _simpleAsm = simpleNode;
    }

    SequenceEle::SequenceEle(FlowBlockBase *fbBase) {
        assert(fbBase != nullptr);
        _subBlock  = fbBase;
    }

    void SequenceEle::genHardware() {

        assert( (_simpleAsm != nullptr) ^ (_subBlock != nullptr));

        ///// it is the basic assignment
        if (_simpleAsm != nullptr){
            stReg     = new StateReg();
            stateNode = stReg->generateStateNode();
            _simpleAsm->addDependState(stReg, BITWISE_AND);
            _simpleAsm->assign();
        }else if (_subBlock != nullptr){
            complexNode = _subBlock->sumarizeBlock();
        }else{
            assert(true);
        }
    }

    void SequenceEle::assignDependDent(SequenceEle *predecessor) const {
        assert(predecessor != nullptr);
        assert((_simpleAsm != nullptr) ^ (_subBlock != nullptr));

        if (_simpleAsm != nullptr){
            stateNode->addDependState(predecessor->getStateFinishIden(), BITWISE_AND);
            stateNode->assign();   ///// assign state node to actual value
        }else if (_subBlock != nullptr){
            complexNode->addDependStateToAllNode(predecessor->getStateFinishIden(), BITWISE_AND);
            complexNode->assignAllNode();
        }else{
            assert(true);
        }

    }

    Operable* SequenceEle::getStateFinishIden() const {
        assert( (_simpleAsm != nullptr) ^ (_subBlock != nullptr));
        if (_simpleAsm != nullptr){
            return stReg;
        }else if (_subBlock != nullptr){
            return complexNode->exitOpr;
        }
        assert(true);
        return nullptr;
    }

    std::vector<Node *> SequenceEle::getEntranceNodes() {
        assert( (_simpleAsm != nullptr) ^ (_subBlock != nullptr));
        if (_simpleAsm != nullptr){
            return {stateNode};
        }else if (_subBlock != nullptr){
            return complexNode->entranceNodes;
        }
        assert(true);
        return {};
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
        /** generate hardware*/
        for (auto& seqMeta: _subSeqMetas) {
            seqMeta.genHardware();
        }
        /** connect  chain*/
        for (size_t idx = 0; (idx+1) < _subSeqMetas.size(); idx++){
            auto& lhsNodeWrapper = _subSeqMetas[idx];
            auto& rhsNodeWrapper = _subSeqMetas[idx+1];
            rhsNodeWrapper.assignDependDent(&lhsNodeWrapper);
        }
        /** build new result NodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->entranceNodes = _subSeqMetas.begin()->getEntranceNodes();
        resultNodeWrap->exitOpr       = _subSeqMetas.rbegin()->getStateFinishIden();

    }

    void FlowBlockSeq::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockSeq::doPostFunction() {
        onDetachBlock();
    }

}

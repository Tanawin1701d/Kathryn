//
// Created by tanawin on 5/12/2566.
//

#include "seq.h"
#include "model/controller/controller.h"



namespace kathryn{

    FlowBlockSeq::FlowBlockSeq(): FlowBlockBase(SEQUENTIAL) {

    }

    void SeqenceMeta::genHw() {
        assert(!isGenHwYet);
        assert(_simpleAsm != nullptr ^ _subBlock != nullptr);
        isGenHwYet = true;

        /** for simple node*/
        if (_simpleAsm != nullptr){
            /**create state register*/
            auto stReg  = new StateReg();
            nodeWrapper = new NodeWrapper({{stReg->genUpdateEvent()},
                                           stReg->genOutputExpression()});
            /** bind register*/
            _simpleAsm->updateElement->setUpdateState(stReg);
            return;
        }
        nodeWrapper = _subBlock->sumarizeBlock();
    }


    void FlowBlockSeq::addElementInFlowBlock(Node *node) {
        assert(node != nullptr);
        _subSeqMetas.emplace_back(node);
    }

    void FlowBlockSeq::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        _subSeqMetas.emplace_back(subBlock);
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrapper* FlowBlockSeq::sumarizeBlock() {
        assert(!_subSeqMetas.empty());
        return new NodeWrapper({_subSeqMetas[0].getNodeWrapper()->entranceElements,
                                _subSeqMetas[_subSeqMetas.size()-1].getNodeWrapper()->exitExpr
                                });
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
            seqMeta.genHw();
        }
        /** connect  chain*/
        for (size_t idx = 0; (idx+1) < _subSeqMetas.size(); idx++){
            auto lhsNodeWrapper = _subSeqMetas[idx].getNodeWrapper();
            auto rhsNodeWrapper = _subSeqMetas[idx+1].getNodeWrapper();
            lhsNodeWrapper->join(rhsNodeWrapper);
        }

    }

    void FlowBlockSeq::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockSeq::doPostFunction() {
        onDetachBlock();
    }


}

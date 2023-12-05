//
// Created by tanawin on 5/12/2566.
//

#include "seq.h"
#include "model/controller/controller.h"



namespace kathryn{

    void SeqenceMeta::genHw() {
        assert(!isGenHwYet);
        assert(_simpleAsm != nullptr ^ _subBlock != nullptr);
        isGenHwYet = true;

        if (_simpleAsm != nullptr){
            auto stReg = new StateReg();
            auto outExp = new expression();
            *outExp = *stReg;
            nodeWrapper = new NodeWrapper({{stReg->genUpdateEvent()}, outExp});
            return;
        }
        nodeWrapper = _subBlock->sumarizeBlock();
    }


    void Seq::addElementInFlowBlock(Node *node) {
        assert(node != nullptr);
        _subSeqMetas.emplace_back(node);
    }

    void Seq::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        _subSeqMetas.emplace_back(subBlock);
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrapper *Seq::sumarizeBlock() {
        assert(!_subSeqMetas.empty());
        return new NodeWrapper({_subSeqMetas[0].getNodeWrapper()->entranceElements,
                                _subSeqMetas[_subSeqMetas.size()-1].getNodeWrapper()->exitExpr
                                });
    }

    void Seq::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void Seq::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void Seq::buildHwComponent() {
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

    void Seq::doPreFunction() {
        onAttachBlock();
    }

    void Seq::doPostFunction() {
        onDetachBlock();
    }


}

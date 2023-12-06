//
// Created by tanawin on 6/12/2566.
//

#include "ifelse.h"


namespace kathryn{


    FlowBlockIf::FlowBlockIf(expression& cond):FlowBlockBase(IF), _cond(&cond) {}

    void FlowBlockIf::addElementInFlowBlock(Node *node) {
        assert(true); /** this flow type will generate single par or seq block*/
    }

    void FlowBlockIf::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrapper *FlowBlockIf::sumarizeBlock() {
        new NodeWrapper({});
    }

    void FlowBlockIf::onAttachBlock() {
        auto sb = genImplicitSubBlk(PARALLEL);
        sb->onAttachBlock();
    }

    void FlowBlockIf::onDetachBlock() {
        subBlocks[0]->onDetachBlock();
        ////// we will hold this end block will handle it
    }

    void FlowBlockIf::buildHwComponent() {
        assert(elseClass != nullptr);
        /**gen subblock interface*/
        elseNw = elseClass->sumarizeBlock();
        for (auto elifClass: elifClasses){
            elifNws.push_back(elifClass->sumarizeBlock());
        }

        /** attach*/
        expression& accTrue = *_cond;
        for (int i = 0; i < elifNws.size(); i++){
            expression& validCond = (~accTrue) | (*elifConds[i]);
            NodeWrapper({{}, &validCond}).join(elifNws[i], BITWISE_AND);
            accTrue = ((accTrue) | (*elifConds[i]));
        }
        NodeWrapper({{}, &(~accTrue)}).join(elseNw, BITWISE_AND);


    }

    void FlowBlockIf::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockIf::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockIf::addElifElement(FlowBlockElif* elifClass) {
        assert(elifClass != nullptr);
        elifClasses.push_back(elifClass);
    }

    void FlowBlockIf::addElseElement(FlowBlockElse* elseClass) {
        assert(elseClass != nullptr);
        elseClass = elseClass;

    }


}
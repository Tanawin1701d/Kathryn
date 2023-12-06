//
// Created by tanawin on 6/12/2566.
//

#include "cwhile.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/par/par.h"
#include "model/FlowBlock/seq/seq.h"


namespace kathryn{

    FlowBlockCwhile::FlowBlockCwhile(expression& condExpr): _condExpr(&condExpr),
                                                            FlowBlockBase(WHILE) {}

    FlowBlockCwhile::~FlowBlockCwhile() {
        delete resultNodeWrapper;
    }

    void FlowBlockCwhile::addElementInFlowBlock(Node *node) {
        assert(true);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockCwhile::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrapper *FlowBlockCwhile::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockCwhile::onAttachBlock() {
        auto sb = genImplicitSubBlk(PARALLEL);
        sb->onAttachBlock();

    }

    void FlowBlockCwhile::onDetachBlock() {
        assert(isGetFlowBlockYet);
        subBlocks[0]->onDetachBlock();
        ctrl->on_detach_flowBlock(this);

    }

    void FlowBlockCwhile::buildHwComponent() {
        assert(!subBlocks.size() == 1);

        /** build subblock*/
        subBlocks[0]->buildHwComponent();
        NodeWrapper* wrapperOfSubBlock = subBlocks[0]->sumarizeBlock();

        /**to fix*/

        /**join loop if condition is true*/
        NodeWrapper* nextNodeWrapper = new NodeWrapper({{},
                                                        &((*wrapperOfSubBlock->exitExpr) & (*_condExpr))});
        nextNodeWrapper->join(wrapperOfSubBlock);

        /** summarize this block*/
        resultNodeWrapper = new NodeWrapper();
        resultNodeWrapper->entranceElements = wrapperOfSubBlock->entranceElements;
        resultNodeWrapper->exitExpr = &((*wrapperOfSubBlock->exitExpr) & !(*_condExpr));

        /** free memory*/
        delete wrapperOfSubBlock;
        delete nextNodeWrapper;

    }

    void FlowBlockCwhile::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockCwhile::doPostFunction() {
        onDetachBlock();
    }

}
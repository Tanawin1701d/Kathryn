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
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrapper *FlowBlockCwhile::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockCwhile::onAttachBlock() {
        /** determine next flow block*/
        FLOW_BLOCK_TYPE nextFbType = ctrl->get_top_pattern_flow_block_type();
        if (nextFbType == DUMMY_BLOCK){
            nextFbType = PARALLEL;
        }
        /** create subblock*/
        if (nextFbType == PARALLEL){
            toDoPostBlock = new FlowBlockPar();
            toDoPostBlock->doPrePostFunction();
        }else if (nextFbType == SEQUENTIAL){
            toDoPostBlock = new FlowBlockSeq();
            toDoPostBlock->doPrePostFunction();
        }else{
            assert(true); /** can't determine flow type*/
        }



    }

    void FlowBlockCwhile::onDetachBlock() {
        toDoPostBlock->step();
        toDoPostBlock->doPrePostFunction();
        ctrl->on_detach_flowBlock(this);

    }

    void FlowBlockCwhile::buildHwComponent() {
        assert(!subBlocks.size() == 1);

        /** build subblock*/
        subBlocks[0]->buildHwComponent();
        NodeWrapper* wrapperOfSubBlock = subBlocks[0]->sumarizeBlock();

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
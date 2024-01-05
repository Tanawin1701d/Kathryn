//
// Created by tanawin on 6/12/2566.
//

#include "cwhile.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockCwhile::FlowBlockCwhile(Operable& condExpr): _condExpr(&condExpr),
                                                          FlowBlockBase(WHILE) {}

    FlowBlockCwhile::~FlowBlockCwhile() {
        for (auto node: loopNodeWrap->entranceNodes){
            delete node;
        }
    }

    void FlowBlockCwhile::addElementInFlowBlock(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockCwhile::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockCwhile::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockCwhile::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = genImplicitSubBlk(PARALLEL);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockCwhile::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockCwhile::buildHwComponent() {
        assert(subBlocks.size() == 1);
        /** get node wrap */
        subBlockNodeWrap = subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);


        /** copy node and node wrap*/
        loopNodeWrap = new NodeWrap(*subBlockNodeWrap);


        /**assign to result node wrap*/
        /** node wrap no assign because we must sent to upper block*/
        resultNodeWrapper = new NodeWrap();
        exitNode          = new PseudoNode();
        byPassExitNode    = new PseudoNode();
        subBlockExitNode  = new PseudoNode();

        /**
         *
         * result node wrap
         *
         * */

        /** entrance result nodewrap*/
        resultNodeWrapper->transferNodeFrom(subBlockNodeWrap);
        /** in case it is not match at first time*/
        resultNodeWrapper->addEntraceNode(byPassExitNode);


        /**exit node wrap*/
        byPassExitNode->addCondtion(&(!*_condExpr), BITWISE_AND);
        /** in case exit from sublock*/
        subBlockExitNode->addCondtion(&(!*_condExpr), BITWISE_AND);
        subBlockExitNode->addDependNode(subBlockNodeWrap->getExitNode());
        subBlockExitNode->setDependStateJoinOp(BITWISE_AND);
        subBlockExitNode->assign();
        /** pool exit node and put to result node wrap*/
        exitNode->addDependNode(subBlockExitNode);
        exitNode->addDependNode(byPassExitNode);
        exitNode->setDependStateJoinOp(BITWISE_OR);
        exitNode->assign();
        resultNodeWrapper->addExitNode(exitNode);




        /**
         * loop back condition to make block repeat
         * */

        /**assign for loop back assignment*/
        loopNodeWrap->addDependNodeToAllNode(subBlockNodeWrap->getExitNode());
        loopNodeWrap->setDependNodeCond(BITWISE_AND);
        if (subBlockNodeWrap->isThereForceExitNode()) {
            Operable *allowLoopCond = &((*_condExpr) & (!*subBlockNodeWrap->getForceExitNode()->getExitOpr()));
            loopNodeWrap->addConditionToAllNode(allowLoopCond, BITWISE_AND);
        }
        else {
            loopNodeWrap->addConditionToAllNode(_condExpr, BITWISE_AND);
        }
        loopNodeWrap->assignAllNode();
    }

    void FlowBlockCwhile::doPreFunction() {
        onAttachBlock();
    }
    void FlowBlockCwhile::doPostFunction() {
        onDetachBlock();
    }

}
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
        assert(true);
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
        sb->onAttachBlock();
    }

    void FlowBlockCwhile::onDetachBlock() {
        assert(isGetFlowBlockYet);
        subBlocks[0]->onDetachBlock();
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
        auto psuedoNode = new Node();
        psuedoNode->addCondtion(&(!*_condExpr), BITWISE_AND);
        resultNodeWrapper->entranceNodes.push_back(psuedoNode);
        resultNodeWrapper->transferNodeFrom(subBlockNodeWrap);
        resultNodeWrapper->exitOpr = &(((*subBlockNodeWrap->exitOpr  ) & !(*_condExpr)) |
                                       ((*psuedoNode->psudoAssignMeta) & !(*_condExpr) ));


        /**assign for loop back assignment*/
        loopNodeWrap->addDependStateToAllNode(subBlockNodeWrap->exitOpr, BITWISE_AND);
        loopNodeWrap->addConditionToAllNode(_condExpr, BITWISE_AND);
        loopNodeWrap->assignAllNode();
    }

    void FlowBlockCwhile::doPreFunction() {
        onAttachBlock();
    }
    void FlowBlockCwhile::doPostFunction() {
        onDetachBlock();
    }

}
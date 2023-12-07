//
// Created by tanawin on 6/12/2566.
//

#include "cwhile.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockCwhile::FlowBlockCwhile(Operable& condExpr): _condExpr(&condExpr),
                                                            FlowBlockBase(WHILE) {}

    FlowBlockCwhile::~FlowBlockCwhile() {
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

    NodeWrap* FlowBlockCwhile::sumarizeBlock() {
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

        /** get node wrap */
        subBlockNodeWrap = subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);
        loopNodeWrap = new NodeWrap(*subBlockNodeWrap);


        /**assign to result node wrap*/
        resultNodeWrapper = new NodeWrap();
        auto psuedoNode = new Node();
        psuedoNode->condition = &(!*_condExpr);
        resultNodeWrapper->entranceNodes.push_back(psuedoNode);
        for (auto node: loopNodeWrap->entranceNodes){
            resultNodeWrapper->entranceNodes.push_back(node);
        }

        resultNodeWrapper->exitOpr = &(((*subBlockNodeWrap->exitOpr  ) & !(*_condExpr)) |
                                       ((*psuedoNode->psudoAssignMeta) & !(*_condExpr) ));

        /**assign for loop back assignment*/
        for (auto nd : loopNodeWrap->entranceNodes){
            nd->addDependState(subBlockNodeWrap->exitOpr, BITWISE_AND);
            nd->addCondtion( _condExpr, BITWISE_AND);
            nd->assign();
        }
    }

    void FlowBlockCwhile::doPreFunction() {
        onAttachBlock();
    }
    void FlowBlockCwhile::doPostFunction() {
        onDetachBlock();
    }

}
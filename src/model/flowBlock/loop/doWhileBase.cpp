//
// Created by tanawin on 22/4/2567.
//

#include "doWhileBase.h"
#include "model/controller/controller.h"


namespace kathryn {

    FlowBlockDowhile::FlowBlockDowhile(Operable &condExpr,
                                       FLOW_BLOCK_TYPE fbt) :
            _condExpr(&condExpr),
            _purifiedCondExpr(purifyCondition(&condExpr)),
            FlowBlockBase(fbt,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }) {
        assert(_purifiedCondExpr != nullptr);
    }

    FlowBlockDowhile::~FlowBlockDowhile() {
        delete resultNodeWrapper;
        delete exitNode;
    }


    void FlowBlockDowhile::buildHwComponent() {
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);

        /***sub block depend lower deck is assume condition or*/
        subBlockNodeWrap->addDependNodeToAllNode(subBlockNodeWrap->getExitNode(), _purifiedCondExpr);
        if (isThereIntStart()) {
            subBlockNodeWrap->addDependNodeToAllNode(intNodes[INT_START], nullptr);
        }
        /** exit node*/
        //////// no need reset signal
        exitNode = new PseudoNode(1, BITWISE_OR);
        exitNode->addDependNode(subBlockNodeWrap->getExitNode(), &(!(*_purifiedCondExpr)));
        exitNode->assign();
        addSysNode(exitNode);
        ////////////////////////////////////////////////////////////////////
        resultNodeWrapper = new NodeWrap();
        resultNodeWrapper->transferEntNodeFrom(subBlockNodeWrap);
        resultNodeWrapper->addExitNode(exitNode);

    }


    void FlowBlockDowhile::addElementInFlowBlock(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockDowhile::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap *FlowBlockDowhile::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockDowhile::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockDowhile::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockDowhile::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockDowhile::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockDowhile::addMdLog(MdLogVal *mdLogVal) {

        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("exitNode " + exitNode->getMdIdentVal() + " " + exitNode->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrapper->getMdIdentVal() + " " + resultNodeWrapper->getMdDescribe());

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->addMdLog(subLog);

    }

}
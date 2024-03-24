//
// Created by tanawin on 24/3/2567.
//

#include "pipelineBase.h"
#include "model/controller/controller.h"



namespace kathryn{


    FlowBlockPipeBase::FlowBlockPipeBase(FLOW_BLOCK_TYPE fbt,
                                         expression* availExpr,
                                         expression* notifyExpr):
    _availExpr(availExpr),
    _notifyExpr(notifyExpr),
    FlowBlockBase(fbt,{
            {FLOW_ST_BASE_STACK},
            FLOW_JO_SUB_FLOW,
            true
    })
    {
        assert(_availExpr  != nullptr);
        assert(_notifyExpr != nullptr);
        assert(_notifyExpr->getSlice().getSize() == 1);
    }

    void FlowBlockPipeBase::addElementInFlowBlock(Node* node){
        assert(false);
    }
    void FlowBlockPipeBase::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(false);
    }
    void FlowBlockPipeBase::addConFlowBlock(FlowBlockBase* conBlock){
        assert(false);
    }
    void FlowBlockPipeBase::addAbandonFlowBlock(FlowBlockBase* abandonBlock){
        assert(false);
    }


    FlowBlockPipeBase::~FlowBlockPipeBase() {
         /** wait session*/
         delete _upWaitNode;
         delete _waitCheckNode;
         delete _waitNode;
         /** go session*/
         delete _upExitNode;
         delete _fromWaitNode;
         delete _exitNode;
         /**notify session*/
         delete _upNotifyNode;
         delete _notifyNode;
    }

    NodeWrap* FlowBlockPipeBase::sumarizeBlock(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockPipeBase::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeBase::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }



    void FlowBlockPipeBase::buildHwComponent(){
        assert(_availExpr  != nullptr);
        assert(_notifyExpr != nullptr);

        /**wait session*/
        _upWaitNode = new PseudoNode(1);
        _upWaitNode->setDependStateJoinOp(BITWISE_AND);
        _upWaitNode->addCondtion(&(!(*_availExpr)), BITWISE_AND);

        _waitCheckNode = new PseudoNode(1);
        _waitCheckNode->setDependStateJoinOp(BITWISE_AND);
        _waitCheckNode->addCondtion(&(!(*_availExpr)), BITWISE_AND);
        _waitCheckNode->addDependNode(_waitNode);
        _waitCheckNode->assign();

        _waitNode = new StateNode();
        _waitNode->setDependStateJoinOp(BITWISE_OR);
        _waitNode->addDependNode(_upWaitNode);
        _waitNode->addDependNode(_waitCheckNode);
        _waitNode->assign();

        /**skip session*/
        _upExitNode = new PseudoNode(1);
        _upExitNode->setDependStateJoinOp(BITWISE_AND);
        _upExitNode->addCondtion(_availExpr, BITWISE_AND);

        _fromWaitNode = new PseudoNode(1);
        _fromWaitNode->setDependStateJoinOp(BITWISE_AND);
        _fromWaitNode->addCondtion(_availExpr, BITWISE_AND);
        _fromWaitNode->addDependNode(_waitNode);
        _fromWaitNode->assign();


        _exitNode = new PseudoNode(1);
        _exitNode->setDependStateJoinOp(BITWISE_OR);
        _exitNode->addDependNode(_fromWaitNode);
        _exitNode->addDependNode(_upExitNode);
        _exitNode->assign();

        /**  build notify node*/
        _upNotifyNode = new PseudoNode(1);
        _upNotifyNode->setDependStateJoinOp(BITWISE_AND);
        /***no condition just let upper assign the finish state*/
        _notifyNode = new PseudoNode(1);
        _notifyNode->setDependStateJoinOp(BITWISE_OR);
        _notifyNode->addDependNode(_waitNode);
        _notifyNode->addDependNode(_upNotifyNode);
        _notifyNode->assign();

        /****reuslt node wrap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->addEntraceNode(_upWaitNode);
        _resultNodeWrap->addEntraceNode(_upExitNode);
        _resultNodeWrap->addEntraceNode(_upNotifyNode);
        _resultNodeWrap->addExitNode(_exitNode);
        /**cycle andd force exit is set to -1*/


        ////////// assign notify expression
        *_notifyExpr = *_notifyNode->getExitOpr();

    }

    void FlowBlockPipeBase::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockPipeBase::doPostFunction() {
        onDetachBlock();
    }
}
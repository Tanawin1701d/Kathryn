//
// Created by tanawin on 25/3/2567.
//

#include "pipelineBlock.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockPipeBase::FlowBlockPipeBase():
    FlowBlockBase(PIPE_BLOCK,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_SUB_FLOW,
                          true
                  }),
    _isGetRecvPipe(false),
    _isGetSendPipe(false),
    _recvPipe(Pipe(false)),
    _sendPipe(Pipe(false))
    {}


    FlowBlockPipeBase::FlowBlockPipeBase(Pipe &recvPipe, Pipe &sendPipe):
    FlowBlockBase(PIPE_BLOCK,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_SUB_FLOW,
                          true
                  }),
    _isGetRecvPipe(true),
    _isGetSendPipe(true),
    _recvPipe(recvPipe),
    _sendPipe(sendPipe)

    {}

    FlowBlockPipeBase::~FlowBlockPipeBase(){
        delete _upNode;
        delete _jointNode;
        delete _exitNode;
        delete _resultNodeWrap;
    }

    void FlowBlockPipeBase::addElementInFlowBlock(Node* node){
        assert(false);
    }

    void FlowBlockPipeBase::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        assert(subBlock->getFlowType() == SEQUENTIAL);
        assert(subBlock == _implicitFlowBlock);
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    void FlowBlockPipeBase::addConFlowBlock(FlowBlockBase* conBlock){
        assert(false);
    }

    NodeWrap* FlowBlockPipeBase::sumarizeBlock(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockPipeBase::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
        /** in pipe line we implicit add seq and pipe recv and sender*/
        _implicitFlowBlock = new FlowBlockSeq();
        _implicitFlowBlock->onAttachBlock();
        /*** attach wait Block*/
        mfAssert(_isGetRecvPipe, "recv pipe is not allocated");
        _waitRecvBlock = new FlowBlockPipeBase(PIPE_RECIEVER, _recvPipe);
        _waitRecvBlock->onAttachBlock();
        _waitRecvBlock->onDetachBlock();

    }

    void FlowBlockPipeBase::onDetachBlock(){

        assert(_implicitFlowBlock != nullptr);
        assert(_isGetImplicitFlowBlockYet);

        mfAssert(_isGetSendPipe, "send pipe is not allocated");
        _waitSendBlock = new FlowBlockPipeBase(PIPE_SENDER, _sendPipe);
        _waitSendBlock->onAttachBlock();
        _waitSendBlock->onDetachBlock();

        _implicitFlowBlock->onDetachBlock();
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeBase::buildHwComponent(){
        buildSubHwComponent();

        _impFbNodeWrap = _implicitFlowBlock->sumarizeBlock();

        /** fire start node*/
        _upNode    = new PseudoNode(1);
        _upNode->setDependStateJoinOp(BITWISE_AND);

        _jointNode = new PseudoNode(1);
        _jointNode->setDependStateJoinOp(BITWISE_OR);
        _jointNode->addDependNode(_upNode);
        _jointNode->addDependNode(_impFbNodeWrap->getExitNode());
        _jointNode->assign();

        /** fire start subBlock*/
        _impFbNodeWrap->addDependNodeToAllNode(_jointNode);
        _impFbNodeWrap->assignAllNode();

        /** fire exit node*/
        _exitNode  = new DummyNode(&_make<Val>("pipeBlockExit", 1,0));

        /** manage result node warap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->addEntraceNode(_upNode);
        _resultNodeWrap->addExitNode(_exitNode);

    }

    void FlowBlockPipeBase::setRecvPipe(Pipe recvPipe){
        _isGetRecvPipe = true;
        mfAssert(recvPipe._isAlloc, "recv Pipe is not allocated");
        _recvPipe = recvPipe;
    }

    void FlowBlockPipeBase::setSendPipe(Pipe sendPipe){
        _isGetSendPipe = true;
        mfAssert(sendPipe._isAlloc, "send Pipe is not allocated");
        _sendPipe = sendPipe;
    }

    void FlowBlockPipeBase::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockPipeBase::doPostFunction() {
        onDetachBlock();
    }



}
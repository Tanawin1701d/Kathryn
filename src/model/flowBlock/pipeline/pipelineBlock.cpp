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
                  })
    {}


    FlowBlockPipeBase::FlowBlockPipeBase(Pipe* recvPipe, Pipe* sendPipe):
    FlowBlockBase(PIPE_BLOCK,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_SUB_FLOW,
                          true
                  }),
    _recvPipe(recvPipe),
    _sendPipe(sendPipe)

    {}

    FlowBlockPipeBase::~FlowBlockPipeBase(){
        delete _joinNode;
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
        assert(!_isGetImplicitFlowBlockYet);
        _isGetImplicitFlowBlockYet = true;
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
        _waitRecvBlock = new FlowBlockPipeCom(PIPE_RECIEVER);
        _waitRecvBlock->onAttachBlock();
        _waitRecvBlock->onDetachBlock();

    }

    void FlowBlockPipeBase::onDetachBlock(){

        assert(_implicitFlowBlock != nullptr);


        _waitSendBlock = new FlowBlockPipeCom(PIPE_SENDER);
        _waitSendBlock->onAttachBlock();
        _waitSendBlock->onDetachBlock();

        _implicitFlowBlock->onDetachBlock();
        assert(_isGetImplicitFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeBase::buildHwMaster(){
        /** build lower deck*/
        fillIntRstSignalToChild();

        /** set the pipe first*/
        mfAssert(_recvPipe != nullptr, "flow block pipe(recv) doesn't get pipe metadata");
        mfAssert(_sendPipe != nullptr, "flow block pipe(send) doesn't get pipe metadata");
        _waitRecvBlock->setPipe(_recvPipe);
        _waitSendBlock->setPipe(_sendPipe);

        buildSubHwComponent();
        /** we so sure now that all sub  Block is ready*/
        genIntNode();
        buildHwComponent();
    }

    void FlowBlockPipeBase::buildHwComponent(){

        /** then build sub hardware component**/

        _impFbNodeWrap = _implicitFlowBlock->sumarizeBlock();

        /** initialize node*/
        _joinNode = new PseudoNode(1, BITWISE_OR);
        addSysNode(_joinNode);
        _joinNode->setInternalIdent("pipJoinNode_" + std::to_string(getGlobalId()));
        _joinNode->addDependNode(_impFbNodeWrap->getExitNode(), nullptr);
        if (isThereIntStart()){
            _joinNode->addDependNode(intNodes[INT_START], nullptr);
        }
            ///// it is not assign because it let upper block assign
        /** impFbNodeWrap depend on joinNode*/
        _impFbNodeWrap->addDependNodeToAllNode(_joinNode, nullptr);
        _impFbNodeWrap->assignAllNode();


        /** fire exit node*/
        _exitNode  = new DummyNode(&_make<Val>("pipeBlockExit", 1,0));
        addSysNode(_exitNode);

        /** manage result node warap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->addEntraceNode(_joinNode);
        _resultNodeWrap->addExitNode(_exitNode);

    }

    void FlowBlockPipeBase::setRecvPipe(Pipe* recvPipe){
        assert(recvPipe != nullptr);
        _recvPipe = recvPipe;
    }

    void FlowBlockPipeBase::setSendPipe(Pipe* sendPipe){
        assert(sendPipe != nullptr);
        _sendPipe = sendPipe;
    }

    void FlowBlockPipeBase::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockPipeBase::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockPipeBase::addMdLog(MdLogVal *mdLogVal) {

        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");

        mdLogVal->addVal("jointNode " +
                                 _joinNode->getMdIdentVal() + "  " +
                                 _joinNode->getMdDescribe());

        mdLogVal->addVal("resultNode wrap is " +
                              _resultNodeWrap->getMdIdentVal() +
                              " " + _resultNodeWrap->getMdDescribe());

        mdLogVal->addVal("implicitSubBlock");
        auto subLog = mdLogVal->makeNewSubVal();
        _implicitFlowBlock->addMdLog(subLog);

    }

}
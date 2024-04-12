//
// Created by tanawin on 24/3/2567.
//

#include "pipelineCom.h"
#include "model/controller/controller.h"
#include "pipelineBlock.h"


namespace kathryn{


    FlowBlockPipeCom::FlowBlockPipeCom(FLOW_BLOCK_TYPE fbt,
                                       Pipe* pipe):
    FlowBlockBase(fbt,{
            {FLOW_ST_BASE_STACK},
            FLOW_JO_SUB_FLOW,
            true
    }),
    _pipe(pipe)
    {

        assert(_pipe != nullptr);
        assert( (fbt == PIPE_SENDER) || (fbt == PIPE_RECIEVER) );

    }

    FlowBlockPipeCom::FlowBlockPipeCom(FLOW_BLOCK_TYPE fbt):
            FlowBlockBase(fbt,{
                    {FLOW_ST_BASE_STACK},
                    FLOW_JO_SUB_FLOW,
                    true
            }){

        assert( (fbt == PIPE_SENDER) || (fbt == PIPE_RECIEVER) );

    }

    void FlowBlockPipeCom::addElementInFlowBlock(Node* node){
        assert(false);
    }
    void FlowBlockPipeCom::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(false);
    }
    void FlowBlockPipeCom::addConFlowBlock(FlowBlockBase* conBlock){
        assert(false);
    }
    void FlowBlockPipeCom::addAbandonFlowBlock(FlowBlockBase* abandonBlock){
        assert(false);
    }


    FlowBlockPipeCom::~FlowBlockPipeCom() {
         delete _resultNodeWrap;
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

    NodeWrap* FlowBlockPipeCom::sumarizeBlock(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockPipeCom::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeCom::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }



    void FlowBlockPipeCom::buildHwComponent(){

        mfAssert(_pipe != nullptr,
                 "pipeline Com doesn't have pipe meta data");
        mfAssert(_interruptNode[INTR_TYPE_START] == nullptr,
                 "pipeline com doesn't support pipe interrupt start signal");

        expression* checkToGoSignal = getFlowType() == PIPE_SENDER ?
                                      _pipe->_slaveReadyToRecv :
                                      _pipe->_masterReadyToSend;

        expression* writeYourStatus = getFlowType() == PIPE_SENDER ?
                                      _pipe->_masterReadyToSend:
                                      _pipe->_slaveReadyToRecv;

        std::string identHelper = getFlowType() == PIPE_SENDER ? "send" : "recv";

        /**
         * no need to build sub component because there is no sub component
         * **/

        /**wait session*/
        _upWaitNode    = new PseudoNode(1);
        _waitCheckNode = new PseudoNode(1);
        _waitNode = new StateNode();



        _upWaitNode->setDependStateJoinOp(BITWISE_AND);
        _upWaitNode->addCondtion(&(!(*checkToGoSignal)), BITWISE_AND);
        _upWaitNode->setInternalIdent("pipCom" + identHelper + "waitUp" + std::to_string(getGlobalId()));

        _waitCheckNode->setDependStateJoinOp(BITWISE_AND);
        _waitCheckNode->addCondtion(&(!(*(checkToGoSignal))), BITWISE_AND);
        _waitCheckNode->addDependNode(_waitNode);
        _waitCheckNode->setInternalIdent("pipCom" + identHelper + "waitCheck" + std::to_string(getGlobalId()));
        _waitCheckNode->assign();

        _waitNode->setDependStateJoinOp(BITWISE_OR);
        _waitNode->addDependNode(_upWaitNode);
        _waitNode->addDependNode(_waitCheckNode);
        if (_interruptNode[INTR_TYPE_RESET] != nullptr){
            if (getFlowType() == PIPE_SENDER){
                _waitNode->addResetIntNode(_interruptNode[INTR_TYPE_RESET]);
            }else if (getFlowType() == PIPE_RECIEVER){
                _upWaitNodeFromReset = new PseudoNode(1);
                _upWaitNodeFromReset->setDependStateJoinOp(BITWISE_AND);
                _upWaitNodeFromReset->addCondtion(&(!(*(checkToGoSignal))), BITWISE_AND);
                _upWaitNodeFromReset->addDependNode(_interruptNode[INTR_TYPE_RESET]);
                _upWaitNodeFromReset->setInternalIdent("pipCom" + identHelper + "waitCheckFromRst" + std::to_string(getGlobalId()));
                _upWaitNodeFromReset->assign();
                _waitNode->addDependNode(_upWaitNodeFromReset);
            }
        }
        _waitNode->setInternalIdent("pipCom" + identHelper + "wait" + std::to_string(getGlobalId()));
        _waitNode->assign();

        /**skip session*/
        _upExitNode   = new PseudoNode(1);
        _fromWaitNode = new PseudoNode(1);
        _exitNode     = new PseudoNode(1);

        _upExitNode->setDependStateJoinOp(BITWISE_AND);
        _upExitNode->addCondtion(checkToGoSignal, BITWISE_AND);
        _upExitNode->setInternalIdent("pipCom" + identHelper + "upExit" + std::to_string(getGlobalId()));

        _fromWaitNode->setDependStateJoinOp(BITWISE_AND);
        _fromWaitNode->addCondtion(checkToGoSignal, BITWISE_AND);
        _fromWaitNode->addDependNode(_waitNode);
        _fromWaitNode->setInternalIdent("pipCom" + identHelper + "ExitfromWait" + std::to_string(getGlobalId()));
        _fromWaitNode->assign();


        _exitNode->setDependStateJoinOp(BITWISE_OR);
        _exitNode->addDependNode(_fromWaitNode);
        _exitNode->addDependNode(_upExitNode);
        if (_interruptNode[INTR_TYPE_RESET] != nullptr){
            if (getFlowType() == PIPE_RECIEVER){
                _upExitNodeFromReset = new PseudoNode(1);
                _upExitNodeFromReset->setDependStateJoinOp(BITWISE_AND);
                _upExitNodeFromReset->addCondtion(checkToGoSignal, BITWISE_AND);
                _upExitNodeFromReset->addDependNode(_interruptNode[INTR_TYPE_RESET]);
                _upExitNodeFromReset->setInternalIdent("pipCom" + identHelper + "waitCheckFromRst" + std::to_string(getGlobalId()));
                _upExitNodeFromReset->assign();
                _waitNode->addDependNode(_upWaitNodeFromReset);
            }
        }
        _exitNode->setInternalIdent("pipCom" + identHelper + "exitNode" + std::to_string(getGlobalId()));
        _exitNode->assign();

        /**
         *
         * build notify node
         *
         * */
        _upNotifyNode   = new PseudoNode(1);
        _notifyNode     = new PseudoNode(1);
        _resultNodeWrap = new NodeWrap();


        _upNotifyNode->setDependStateJoinOp(BITWISE_AND);
        /***no condition just let upper assign the finish state*/

        _notifyNode->setDependStateJoinOp(BITWISE_OR);
        _notifyNode->addDependNode(_waitNode);
        _notifyNode->addDependNode(_upNotifyNode);
        _notifyNode->assign();

        /****reuslt node wrap*/
        _resultNodeWrap->addEntraceNode(_upWaitNode);
        _resultNodeWrap->addEntraceNode(_upExitNode);
        _resultNodeWrap->addEntraceNode(_upNotifyNode);
        if (getFlowType() == PIPE_SENDER) {
            _resultNodeWrap->addExitNode(suppressExitOprWithRst(_exitNode));
        }else{
            _resultNodeWrap->addExitNode(_exitNode);
        }
        /**cycle andd force exit is set to -1*/


        ////////// assign notify expression
        *writeYourStatus = *_notifyNode->getExitOpr();

    }

    void FlowBlockPipeCom::setPipe(Pipe* pipEle){
        assert(_pipe  == nullptr);
        assert(pipEle != nullptr);
        _pipe = pipEle;
    }

    /**loop st macro*/

    void FlowBlockPipeCom::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockPipeCom::doPostFunction() {
        onDetachBlock();
    }

    /** debug*/
    void FlowBlockPipeCom::addMdLog(MdLogVal* mdLogVal){
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("_upwaitCheckNode " +
                         _upWaitNode->getMdIdentVal() + " " +
                         _upWaitNode->getMdDescribe()
                         );
        mdLogVal->addVal("_upExitNode " +
                             _upExitNode->getMdIdentVal() + " " +
                             _upExitNode->getMdDescribe()
        );
        mdLogVal->addVal("_notifyNode " +
                                 _notifyNode->getMdIdentVal() + " " +
                                 _notifyNode->getMdDescribe()
        );
    }

    /** sim start cur cycle*/

    void FlowBlockPipeCom::simStartCurCycle(){
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();

        /** check running status*/
        bool isStateRunning = false;
        _waitNode->simStartCurCycle();
        isStateRunning |= _waitNode->isBlockOrNodeRunning();

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }

        /** do sim all to maintain policy*/
        _upWaitNode   ->simStartCurCycle();
        _waitCheckNode->simStartCurCycle();

        _upExitNode   ->simStartCurCycle();
        _fromWaitNode ->simStartCurCycle();
        _exitNode     ->simStartCurCycle();

        _upNotifyNode ->simStartCurCycle();
        _notifyNode   ->simStartCurCycle();
    }

    void FlowBlockPipeCom::simExitCurCycle(){
        unSetSimStatus();
        unsetBlockOrNodeRunning();

        _upWaitNode   ->simExitCurCycle();
        _waitCheckNode->simExitCurCycle();
        _waitNode     ->simExitCurCycle();

        _upExitNode   ->simExitCurCycle();
        _fromWaitNode ->simExitCurCycle();
        _exitNode     ->simExitCurCycle();

        _upNotifyNode ->simExitCurCycle();
        _notifyNode   ->simExitCurCycle();

    }


}
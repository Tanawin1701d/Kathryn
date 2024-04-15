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
         delete upCondNode;
         delete _waitNode;
         delete endNode;
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
        assert(!isThereIntStart());

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

        /*** initialize node*/
        upCondNode = new PseudoNode(1, BITWISE_OR);
        _waitNode  = new StateNode();
        _waitNode->setInternalIdent("pipCom" + identHelper + "waitNode" + std::to_string(getGlobalId()));
        fillIntResetToNodeIfThere(_waitNode);
        endNode         = new PseudoNode(1, BITWISE_OR);
        _resultNodeWrap = new NodeWrap();

        /*** wait node*/
        _waitNode->addDependNode(upCondNode, &!(*checkToGoSignal));
        _waitNode->addDependNode(_waitNode, &!(*checkToGoSignal));
        _waitNode->assign();
        /*** endNode*/
        endNode->addDependNode(_waitNode, checkToGoSignal);
        endNode->addDependNode(upCondNode, checkToGoSignal);
        endNode->assign();
        /***/
        _resultNodeWrap->addEntraceNode(upCondNode);
        _resultNodeWrap->addExitNode   (endNode);

        ////////// assign notify expression
        *writeYourStatus = (*upCondNode->getExitOpr()) | (*_waitNode->getExitOpr());

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
                                 upCondNode->getMdIdentVal() + " " +
                                 upCondNode->getMdDescribe()
                         );
        mdLogVal->addVal("_upExitNode " +
                                 endNode->getMdIdentVal() + " " +
                                 endNode->getMdDescribe()
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
        upCondNode->simStartCurCycle();
        _waitNode ->simStartCurCycle();
        endNode   ->simStartCurCycle();
    }

    void FlowBlockPipeCom::simExitCurCycle(){
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        upCondNode->simExitCurCycle();
        _waitNode ->simExitCurCycle();
        endNode   ->simExitCurCycle();

    }


}
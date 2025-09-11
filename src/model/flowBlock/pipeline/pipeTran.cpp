//
// Created by tanawin on 5/3/2025.
//

#include "pipeTran.h"

#include <utility>
#include "pipePooler.h"
#include "model/controller/controller.h"


namespace kathryn{




    ///////// TRAN BLOCK

    FlowBlockPipeTran::FlowBlockPipeTran():
    FlowBlockBase(PIPE_TRAN,
    {
            {FLOW_ST_BASE_STACK},
            FLOW_JO_SUB_FLOW,
            true
        }),
    basicStNodeExitExpr(new expression(1)){}


    FlowBlockPipeTran::FlowBlockPipeTran(const std::string& targetPipeName,
                                         Operable* customCond):
    FlowBlockPipeTran(){
        addTranMeta(targetPipeName, customCond);
    }

    FlowBlockPipeTran::FlowBlockPipeTran(std::string  assignActivateCond):
    FlowBlockPipeTran(assignActivateCond, nullptr){}


    FlowBlockPipeTran::~FlowBlockPipeTran(){
        delete condNode;
        delete exitNode;
        delete basicStNode;
    }

    void FlowBlockPipeTran::addElementInFlowBlock(Node* node){
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockPipeTran::addSubFlowBlock(FlowBlockBase* subBlock){
        mfAssert(false, "pipTran blk support only basic assigment and zif block not subblock");
    }

    void FlowBlockPipeTran::addConFlowBlock(FlowBlockBase* conBlock){
        mfAssert(false, "pipTran blk support only basic assigment and zif block not conblock");
    }

    void FlowBlockPipeTran::addAbandonFlowBlock(FlowBlockBase* abandonBlock){
        FlowBlockBase::addAbandonFlowBlock(abandonBlock);
    }

    NodeWrap* FlowBlockPipeTran::sumarizeBlock(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockPipeTran::addTranMeta(const std::string& targetPipeName,
                                        Operable* userCond){

        _pipeTranMetas.push_back({targetPipeName, userCond, nullptr});
        _pipeTranMetas.back().createActivateSignal();

    }


    /** add sync component*/

    void FlowBlockPipeTran::addSyncMeta(const std::string& offerName, const std::string& choiceName, Operable* activateIf){
        _pipeSyncMetas.push_back(PipeSyncMeta{offerName, choiceName,activateIf});
        _pipeSyncMetas.back().createPredefSignal();
    }

    void FlowBlockPipeTran::addSyncMeta(const std::string& offerName, Operable* activateIf){
        addSyncMeta(offerName, _hostPipe->getPipeName(), _activateSignal);
    }


    void FlowBlockPipeTran::addSyncMetas(const std::vector<std::string>& tranMetas){
        for (const std::string& offerName: tranMetas){
            addSyncMeta(offerName, nullptr);
        }
    }

    void FlowBlockPipeTran::assignTargetPipeActivateCond(){
        Operable* notRstSig = isThereIntRst() ? &(~(*intNodes[INT_RESET]->getExitOpr())): nullptr;
        for (PipeTranMeta& tranMeta: _pipeTranMetas){
            tranMeta.assignActivateSignal(notRstSig);
        }
    }

    void FlowBlockPipeTran::assignOfferActivateCond(){
        for (PipeSyncMeta& _pipeSyncMetas: _pipeSyncMetas){
            _pipeSyncMetas.assignOfferSignal();
        }
    }


    Operable* FlowBlockPipeTran::joinTargetPipeActivateCond() const{
        mfAssert(!_pipeTranMetas.empty(), "tran cannot have empty");

        Operable* tranAck = _pipeTranMetas[0]._activateSignal;
        for (int i = 1; i < _pipeTranMetas.size(); i++){
            assert(_pipeTranMetas[i]._activateSignal != nullptr);
            switch (_tranPolicy){
            case BITWISE_OR : {tranAck = &((*tranAck) | (*_pipeTranMetas[i]._activateSignal));break;}
            case BITWISE_AND: {tranAck = &((*tranAck) & (*_pipeTranMetas[i]._activateSignal));break;}
            default         : {mfAssert(false, "tran pol does not support tranPol = " + _tranPolicy);}
            }
        }

        return tranAck;
    }

    Operable* FlowBlockPipeTran::joinTargetOfferActivateCond() const{
        if (_pipeSyncMetas.empty()){
            return nullptr;    ///// incase there is no offer need,
            ///// it must return nullptr
        }

        Operable* tranAck = _pipeSyncMetas[0]._intendOfferSignal;
        for (int i = 1; i < _pipeSyncMetas.size(); i++){
            assert(_pipeSyncMetas[i]._intendOfferSignal != nullptr);
            tranAck = &((*tranAck) & (*_pipeSyncMetas[i]._intendOfferSignal));
        }
        return tranAck;
    }


    Operable* FlowBlockPipeTran::getReadySignal(const std::string& pipeTarget) const{

        for (const PipeTranMeta& tranMeta: _pipeTranMetas){
            if (tranMeta._targetPipeName == pipeTarget){
                return &((*basicStNodeExitExpr) & (*tranMeta._activateSignal));
            }
        }
        mfAssert(false, "can't find tran's ready signal for pipeTarget -> " + pipeTarget);
        return nullptr; //// prevent clion warning
    }

    std::vector<std::string> FlowBlockPipeTran::getTranTargetNames() const{
        std::vector<std::string> resultNames;
        resultNames.reserve(_pipeTranMetas.size());
        for (const PipeTranMeta& tranMeta: _pipeTranMetas){
            resultNames.push_back(tranMeta._targetPipeName);
        }
        return resultNames;
    }

    FlowBlockPipeBase* FlowBlockPipeTran::getHostPipeFromCtrl(){

        bool isTopFlowBlockCorrect = ctrl->isTopOfStackBelongToTheSameModule(FLOW_ST_PIP_BLK, FLOW_ST_BASE_STACK);

        if (!isTopFlowBlockCorrect){
            return nullptr;
        }

        FlowBlockBase* fbBase = ctrl->getTopFlowBlockBase(FLOW_ST_PIP_BLK);;
        assert(fbBase->getFlowType() == PIPE_BLOCK);

        return static_cast<FlowBlockPipeBase*>(fbBase);
    }



    /// FLOW BLOCK
    void FlowBlockPipeTran::onAttachBlock(){
        _hostPipe = getHostPipeFromCtrl();
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeTran::onDetachBlock(){
        ////// register the pipe pooler
        PipePooler* pipePooler = getPipePooler();
        pipePooler->addTranBlk(this);
        ctrl->on_detach_flowBlock(this);
    }

    /// build hardware conponent
    void FlowBlockPipeTran::buildHwComponent(){
        assert(_subBlocks.empty());
        assert(_conBlocks.empty());
        assert(_hostPipe != nullptr);
        mfAssert(!_pipeTranMetas.empty(), "tranBlock: " + _hostPipe->getPipeName() + " have empty target pipe");
        ///// connect to pooler to get each signal target pipe is ready to transfer
        assignTargetPipeActivateCond();
        assignOfferActivateCond();
        ///// the signal that tell it is ready to go
        _offerReadySignal      = joinTargetOfferActivateCond();
        _targetPipeReadySignal = joinTargetPipeActivateCond();

        if (_offerReadySignal == nullptr){
            ////// it can be no offer
            _activateSignal = _targetPipeReadySignal;
        }else{
            _activateSignal = &((*_targetPipeReadySignal ) & (*_offerReadySignal));
        }



        /////// create node
        condNode       = new PseudoNode(1, BITWISE_OR);
        exitNode       = new PseudoNode(1, BITWISE_OR);
        basicStNode    = new StateNode();
        basicStNode->setInternalIdent("tranBasic_" + std::to_string(getGlobalId()));
        fillIntResetToNodeIfThere(basicStNode);
        if (isThereIntStart()){
            basicStNode->addDependNode(intNodes[INT_START], nullptr);
        }
        /////// add node dependency
        for (auto nd: _basicNodes){
            assert(nd->getNodeType() == ASM_NODE);
            auto* asmNode = (AsmNode*)nd;
            asmNode->addPreCondition(_activateSignal, BITWISE_AND);
            basicStNode->addSlaveAsmNode(asmNode);
        }
        condNode   ->addDependNode(basicStNode, nullptr);
        exitNode   ->addDependNode(condNode   , _activateSignal);
        basicStNode->addDependNode(condNode   , &(~(*_activateSignal)));


        /////// systemNode
        addSysNode(condNode);
        addSysNode(exitNode);
        addSysNode(basicStNode);

        ////// proxy signal
        (*basicStNodeExitExpr) = (*basicStNode->getExitOpr());
        /////// assign node
        condNode   ->assign();
        exitNode   ->assign();
        //////// basicStNode->assign(); basic node is not fin yet

        /////// result node wrap

        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(basicStNode);
        resultNodeWrap->addExitNode(exitNode);
    }

    void FlowBlockPipeTran::addMdLog(MdLogVal* mdLogVal){
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal( "condNode "    + condNode->getMdIdentVal()    + " " + condNode->getMdDescribe());
        mdLogVal->addVal( "exitNode "    + exitNode->getMdIdentVal()    + " " + exitNode->getMdDescribe());
        mdLogVal->addVal( "basicStNode " + basicStNode->getMdIdentVal() + " " + basicStNode->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrap->getMdIdentVal() + " " + resultNodeWrap->getMdDescribe());
    }

    /// LOOP STMACRO
    void FlowBlockPipeTran::doPreFunction(){
        onAttachBlock();
    }

    void FlowBlockPipeTran::doPostFunction(){
        onDetachBlock();
    }

}
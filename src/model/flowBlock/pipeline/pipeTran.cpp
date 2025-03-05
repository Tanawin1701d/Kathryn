//
// Created by tanawin on 5/3/2025.
//

#include "pipeTran.h"

#include <utility>
#include "pipePooler.h"
#include "model/controller/controller.h"


namespace kathryn{

    FlowBlockPipeTran::FlowBlockPipeTran(std::string  targetPipeName):
    FlowBlockBase(PIPE_TRAN,
    {
            {FLOW_ST_BASE_STACK},
            FLOW_JO_SUB_FLOW,
            true
        }),
    _targetPipe(std::move(targetPipeName)){
         PipePooler* pipePooler = getPipePooler();
         pipePooler->addTranBlk(this);
        readySignal = new expression(1);
    }

    FlowBlockPipeTran::FlowBlockPipeTran(const std::string& targetPipeName,
                                         Operable& customCond):
    FlowBlockPipeTran(targetPipeName){
        _customCond = &customCond;
    }


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

    Operable* FlowBlockPipeTran::createActivateCond() const{
        PipePooler* pipePooler = getPipePooler();
        assert(pipePooler != nullptr);
        Operable* pipeReadySignal = pipePooler->getPipeReadySignal(getTargetName());
        /////// in case pipeTranWhen  addtional condition is required
        Operable* readyToGo = nullptr;
        if (_customCond == nullptr){
            readyToGo = pipeReadySignal;
        }else{
            readyToGo = &((*pipeReadySignal) & (*_customCond));
        }
        return readyToGo;
    }
    /**build ready signal */
    void FlowBlockPipeTran::buildReadySignal(){
        (*readySignal) = *exitNode->getExitOpr();
    }

    /// FLOW BLOCK
    void FlowBlockPipeTran::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeTran::onDetachBlock(){
        ctrl->on_detach_flowBlock(this);
    }

    /// build hardware conponent
    void FlowBlockPipeTran::buildHwComponent(){
        assert(_subBlocks.empty());
        assert(_conBlocks.empty());

        Operable* readyToGo = createActivateCond();

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
            asmNode->addPreCondition(readyToGo, BITWISE_AND);
            basicStNode->addSlaveAsmNode(asmNode);
        }
        condNode   ->addDependNode(basicStNode, nullptr);
        exitNode   ->addDependNode(condNode   , readyToGo);
        basicStNode->addDependNode(condNode, &(~(*readyToGo)));


        /////// systemNode
        addSysNode(condNode);
        addSysNode(exitNode);
        addSysNode(basicStNode);

        /////// assign node
        condNode   ->assign();
        exitNode   ->assign();
        //////// basicStNode->assign(); basic node is not fin yet

        /////// result node wrap

        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNode(basicStNode);
        resultNodeWrap->addExitNode(exitNode);

        buildReadySignal();

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
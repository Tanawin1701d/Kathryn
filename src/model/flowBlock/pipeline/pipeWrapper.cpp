//
// Created by tanawin on 25/3/2567.
//

#include "pipeWrapper.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockPipeWrapper::FlowBlockPipeWrapper():
    FlowBlockBase(PIPE_WRAPPER,
                  {
                          {FLOW_ST_BASE_STACK, FLOW_ST_PIP_WRAP},
                          FLOW_JO_SUB_FLOW,
                          true
                  }){}

    FlowBlockPipeWrapper::~FlowBlockPipeWrapper(){
        delete _dummyExitNode;
        delete _resultNodeWrap;
    }

    void FlowBlockPipeWrapper::addElementInFlowBlock(Node* node) {
        mfAssert(false,
                 "in pipe wrapper only support pip block"
        );
    }

    void FlowBlockPipeWrapper::addSubFlowBlock(FlowBlockBase* subBlock) {
        assert(subBlock != nullptr);
        mfAssert(subBlock->getFlowType() == PIPE_BLOCK,
                 "in pipe wrapper only support pip block"
                 );
        auto* cvtPipBlock = (FlowBlockPipeBase*) subBlock;
        _insidePipBlks.push_back(cvtPipBlock);
        FlowBlockBase::addSubFlowBlock(subBlock);

    }

    void FlowBlockPipeWrapper::addConFlowBlock(FlowBlockBase *conBlock) {
        mfAssert(false,
                 "in pipe wrapper only support pip block"
        );
    }

    NodeWrap* FlowBlockPipeWrapper::sumarizeBlock(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    expression& FlowBlockPipeWrapper::getNextPipBlockReadySignal(){
        /** due to this getting pipe block is not fill to the vector*/
        int declareBlock = _insidePipBlks.size();
        _userCheckNextSignals.emplace_back(declareBlock);
        return *(_userCheckNextSignals.rbegin()->expr);
    }



    void FlowBlockPipeWrapper::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeWrapper::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeWrapper::buildHwMaster(){
        /** build lower deck*/
        fillIntRstSignalToChild();

        /**make pip communication*/
        assert(!_insidePipBlks.empty());
        _pipComs = makePipes(_insidePipBlks.size()+1); //// we must +1 for the end of pip
        _pipComs[0]->setDummyStartPipe();
        _pipComs[_pipComs.size()-1]->setDummyStopPipe();
        /**set pip meta to block*/
        for (int blkId = 0; blkId < _insidePipBlks.size(); blkId++){
            _insidePipBlks[blkId]->setRecvPipe(_pipComs[blkId]);
            _insidePipBlks[blkId]->setSendPipe(_pipComs[blkId+1]);
        }
        /**set user check next signal*/
        for (auto& userCheckNextSignal: _userCheckNextSignals){
            userCheckNextSignal.connectSignal(_pipComs);
        }

        buildSubHwComponent();
        /** we so sure now that all sub  Block is ready*/
        genIntNode();
        buildHwComponent();
    }

    void FlowBlockPipeWrapper::buildHwComponent() {
        /** no need to build sub element*/

        /**build hardware and get sumarize result*/
        for (auto _insidePipBlk : _insidePipBlks){
            _nwOfPipBlks.push_back(_insidePipBlk->sumarizeBlock());
        }

        /** result node wrap management*/
        _resultNodeWrap = new NodeWrap();
        for (auto nwOfPipBlk: _nwOfPipBlks){
            assert(nwOfPipBlk != nullptr);
            _resultNodeWrap->transferEntNodeFrom(nwOfPipBlk);
        }
        _dummyExitVal = &makeOprVal("dummyExitOfPipeWrapper", 1, 0);
        _dummyExitNode = new DummyNode(_dummyExitVal);
        addSysNode(_dummyExitNode);
        _resultNodeWrap->addExitNode(_dummyExitNode);

    }

    void FlowBlockPipeWrapper::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockPipeWrapper::doPostFunction() {
        onDetachBlock();
    }


    /**
     *
     * debugging stage
     *
     * */

    void FlowBlockPipeWrapper::addMdLog(MdLogVal *mdLogVal) {
            assert(mdLogVal != nullptr);

            mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
            mdLogVal->addVal("there is " + std::to_string(_insidePipBlks.size())+" sub pip block");
            for (auto _insidePipBlk: _insidePipBlks){
                auto subLog = mdLogVal->makeNewSubVal();
                _insidePipBlk->addMdLog(subLog);
            }
    }


    /**
     *
     * user check next signal structure
     *
     * **/



    FlowBlockPipeWrapper::UserCheckNextSignal::UserCheckNextSignal(int pipIdx):
    srcPipId(pipIdx),
    desPipId(pipIdx+1),
    expr(&makeOprProxyExpr("userCheckSignal" + std::to_string(pipIdx), false, 1))
    {}

    void FlowBlockPipeWrapper::UserCheckNextSignal::connectSignal(std::vector<Pipe*> &allPip) {
        assert(desPipId < allPip.size());
        /////// get start block of next block
        (*expr) = *(allPip[desPipId]->_slaveReadyToRecv);
    }
}
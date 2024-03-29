//
// Created by tanawin on 25/3/2567.
//

#include "pipeWrapper.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockPipeWrapper::FlowBlockPipeWrapper():
    FlowBlockBase(PIPE_WRAPPER,
                  {
                          {FLOW_ST_BASE_STACK},
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

    void FlowBlockPipeWrapper::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeWrapper::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeWrapper::buildHwComponent() {
        /** no need to build sub element*/

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
        /**build hardware and get sumarize result*/
        for (auto _insidePipBlk : _insidePipBlks){
            _insidePipBlk->buildHwComponent();
            _nwOfPipBlks.push_back(_insidePipBlk->sumarizeBlock());
        }
        /** result node wrap management*/
        _resultNodeWrap = new NodeWrap();
        for (auto nwOfPipBlk: _nwOfPipBlks){
            assert(nwOfPipBlk != nullptr);
            _resultNodeWrap->transferEntNodeFrom(nwOfPipBlk);
        }
        _dummyExitVal = &_make<Val>("dummyExitOfPipeWrapper", 1, 0);
        _dummyExitNode = new DummyNode(_dummyExitVal);
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
     * start sim cycle
     *
     * */

    void FlowBlockPipeWrapper::simStartCurCycle() {
        if(isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isStateRunning = false;

        for(auto insidePipBlk: _insidePipBlks){
            insidePipBlk->simStartCurCycle();
            isStateRunning |= insidePipBlk->isBlockOrNodeRunning();
        }

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }

        _dummyExitNode->simStartCurCycle();


    }

    void FlowBlockPipeWrapper::simExitCurCycle() {

        unSetSimStatus();
        unsetBlockOrNodeRunning();

        for(auto insidePipBlk: _insidePipBlks){
            insidePipBlk->simExitCurCycle();
        }
        _dummyExitNode->simExitCurCycle();

    }


}
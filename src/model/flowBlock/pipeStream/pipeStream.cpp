//
// Created by tanawin on 7/2/26.
//

#include "pipeStream.h"
#include "model/controller/controller.h"

namespace kathryn{

    /**
     * pipestream flow
     */
    FlowBlockPipeStream::FlowBlockPipeStream():
        FlowBlockBase(PIPE_STREAM,
                      {
                        {FLOW_ST_BASE_STACK},
                        FLOW_JO_SUB_FLOW,
                        true
                      }){}

    FlowBlockPipeStream::~FlowBlockPipeStream(){
        for (auto pipStreamEle: _subStreamEles){
            delete pipStreamEle;
        }
        delete _resultNodeWrap;
        delete _dummyNode;
    }

    void FlowBlockPipeStream::addElementInFlowBlock(Node* node){
        assert(node != nullptr);
        auto seqEle = new SequenceEleBasic(node);
        _subStreamEles.push_back(new StreamEle(seqEle));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockPipeStream::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        auto seqEle = new SequenceEleFlowBlock(subBlock);
        _subStreamEles.emplace_back(new StreamEle(seqEle));
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockPipeStream::sumarizeBlock(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockPipeStream::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeStream::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeStream::buildHwComponent() {
        mfAssert(!_subStreamEles.empty(), "pipestream flow must have at least one element");
        assert(_conBlocks.empty());
        /**generate hardware*/
        std::vector<Node*> allStartNodes;
        int idx = 0;
        for (auto& streamMeta: _subStreamEles){
            ///////// initialize hardware
            streamMeta->setIntReset(intNodes[INT_RESET]); //// set interrupt reset must be set before gennode
            streamMeta->setHoldNode(holdNode);
            streamMeta->genNode(getClockMode());
            streamMeta->setIdentStateId(getGlobalId(),idx);
            streamMeta->addToSystemNodes(_sysNodes);
            idx++;
        }

        for (idx = 0; idx < _subStreamEles.size(); idx++){
            ////////// connect the dependency
            StreamEle* streamMeta = _subStreamEles[idx];
            StreamEle* prevEle = (idx == 0)
                                  ? nullptr: _subStreamEles[idx-1];
            StreamEle* nextEle = (idx == (_subStreamEles.size()-1))
                                  ? nullptr: _subStreamEles[idx+1];
            _subStreamEles[idx]->addSyncDependency(prevEle, nextEle);
            if (isThereIntStart()){
                _subStreamEles[idx]->assignIntStart(intNodes[INT_START]);
            }
            allStartNodes.push_back(streamMeta->getEntranceNode());
        }

        makeVal(dummyPipStreamVal, 1,0);
        _dummyNode = new DummyNode(&dummyPipStreamVal);

        /**result Node wrap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->addEntraceNodes(allStartNodes);
        _resultNodeWrap->addExitNode(_dummyNode);

    }

    std::string FlowBlockPipeStream::getMdDescribe() {

        // std::string ret;
        // int eleCnt = 0;
        // ret += "[ " + FlowBlockBase::getMdIdentVal() + " ]\n";
        // for (auto seqEle: _subSeqMetas){
        //     ret += seqEle->getDescribe() + "\n";
        //     eleCnt++;
        // }
        // ret += getMdDescribeRecur();
        // return ret;
        return "";

    }

    void FlowBlockPipeStream::addMdLog(MdLogVal *mdLogVal) {

        // mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        // for (auto seqEle: _subSeqMetas){
        //     mdLogVal->addVal(seqEle->getDescribe());
        // }
        // if (resultNodeWrap->isThereForceExitNode()){
        //     mdLogVal->addVal("forceExit is " + resultNodeWrap->getForceExitNode()->getMdIdentVal() +
        //                                            "  " +
        //                                            resultNodeWrap->getForceExitNode()->getMdDescribe());
        // }
        //
        // addMdLogRecur(mdLogVal);


    }

    void FlowBlockPipeStream::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockPipeStream::doPostFunction() {
        onDetachBlock();
    }


}

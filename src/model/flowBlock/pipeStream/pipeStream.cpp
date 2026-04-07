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

    void FlowBlockPipeStream::add_basic_node(Node* node){
        assert(node != nullptr);
        auto seqEle = new SequenceEleBasic(node);
        _subStreamEles.push_back(new StreamEle(seqEle));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::add_basic_node(node);
    }

    void FlowBlockPipeStream::add_sub_flow_block(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        auto seqEle = new SequenceEleFlowBlock(subBlock);
        _subStreamEles.emplace_back(new StreamEle(seqEle));
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    NodeWrap* FlowBlockPipeStream::sumarize_block(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockPipeStream::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeStream::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeStream::build_hw_component() {
        mf_assert(!_subStreamEles.empty(), "pipestream flow must have at least one element");
        assert(_con_blocks.empty());
        /**generate hardware*/
        std::vector<Node*> allStartNodes;
        int idx = 0;
        for (auto& streamMeta: _subStreamEles){
            ///////// initialize hardware
            streamMeta->setIntReset(_int_nodes[INT_RESET]); //// set interrupt reset must be set before gennode
            streamMeta->setHoldNode(_hold_node);
            streamMeta->genNode(get_clock_mode());
            streamMeta->setIdentStateId(get_global_id(),idx);
            streamMeta->addToSystemNodes(_sys_nodes);
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
            if (is_there_intr_start()){
                _subStreamEles[idx]->assignIntStart(_int_nodes[INT_START]);
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

    std::string FlowBlockPipeStream::get_md_describe() {

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

    void FlowBlockPipeStream::add_md_log(MdLogVal *mdLogVal) {

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
        on_attach_block();
    }

    void FlowBlockPipeStream::doPostFunction() {
        on_detach_block();
    }


}

//
// Created by tanawin on 7/2/26.
//

#ifndef MODEL_FLOWBLOCK_PIPESTREAM_PIPESTREAM_H
#define MODEL_FLOWBLOCK_PIPESTREAM_PIPESTREAM_H


#include "model/flowBlock/abstract/flowBlockBase.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "streamEle.h"

#define pipStream for(auto kathrynBlock = new FlowBlockPipeStream(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockPipeStream : public FlowBlockBase, public LoopStMacro{
    private:
        std::vector<StreamEle*> _subStreamEles;
        NodeWrap*               _resultNodeWrap = nullptr;
        DummyNode*              _dummyNode      = nullptr;
    public:
        FlowBlockPipeStream();
        ~FlowBlockPipeStream() override;
        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrap* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
    };

}


#endif //MODEL_FLOWBLOCK_PIPESTREAM_PIPESTREAM_H
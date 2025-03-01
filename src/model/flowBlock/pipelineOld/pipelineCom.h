//
// Created by tanawin on 24/3/2567.
//

#ifndef KATHRYN_PIPELINECOM_H
#define KATHRYN_PIPELINECOM_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "pipeMng.h"

#define pipWaitSend(kathrynPipMeta) for(auto kathrynBlock = new FlowBlockPipeCom(PIPE_SENDER,&kathrynPipMeta); \
                                             kathrynBlock->doPrePostFunction();                               \
                                             kathrynBlock->step()){}

#define pipWaitRecv(kathrynPipMeta) for(auto kathrynBlock = new FlowBlockPipeCom(PIPE_RECIEVER,&kathrynPipMeta); \
                                             kathrynBlock->doPrePostFunction();                               \
                                             kathrynBlock->step()){}


namespace kathryn{

    class FlowBlockPipeCom: public FlowBlockBase, public LoopStMacro{

        /**
         *
         *
         * please remind that in this version we will not support
         * force exit node due to complexity purpose
         *
         * **/
        Pipe*         _pipe          = nullptr;
        NodeWrap*    _resultNodeWrap = nullptr;

        PseudoNode* upCondNode = nullptr;
        StateNode*  _waitNode  = nullptr;
        PseudoNode* endNode    = nullptr;

    public:

        explicit FlowBlockPipeCom(FLOW_BLOCK_TYPE fbt,
                                   Pipe* pipe);

        explicit FlowBlockPipeCom(FLOW_BLOCK_TYPE fbt);

        ~FlowBlockPipeCom() override;

        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;
        void addAbandonFlowBlock(FlowBlockBase* abandonBlock) override;

        NodeWrap*   sumarizeBlock() override;
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        void setPipe(Pipe* pipEle);


        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

        /**get debug*/
        void addMdLog(MdLogVal* mdLogVal) override;

    };

}

#endif //KATHRYN_PIPELINECOM_H

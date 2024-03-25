//
// Created by tanawin on 24/3/2567.
//

#ifndef KATHRYN_PIPELINECOM_H
#define KATHRYN_PIPELINECOM_H


#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"
#include "model/FlowBlock/abstract/nodes/node.h"
#include "model/FlowBlock/abstract/nodes/stateNode.h"
#include "pipeMng.h"

#define pipWaitSend(kathrynPipMeta) for(auto kathrynBlock = new FlowBlockPipeCom(PIPE_SENDER,kathrynPipMeta); \
                                             kathrynBlock->doPrePostFunction();                               \
                                             kathrynBlock->step())

#define pipWaitRecv(kathrynPipMeta) for(auto kathrynBlock = new FlowBlockPipeCom(PIPE_RECIEVER,kathrynPipMeta); \
                                             kathrynBlock->doPrePostFunction();                               \
                                             kathrynBlock->step())


namespace kathryn{

    class FlowBlockPipeCom: public FlowBlockBase, public LoopStMacro{

        /**
         *
         *
         * please remind that in this version we will not support
         * force exit node due to complexity purpose
         *
         * **/
        Pipe         _pipe;
        NodeWrap*    _resultNodeWrap = nullptr;

        /**our component*/
        ////// wait session
        PseudoNode*  _upWaitNode    = nullptr; ////// upper node assign to wait
        PseudoNode*  _waitCheckNode = nullptr;
        StateNode*   _waitNode      = nullptr;

        ////// skip session
        PseudoNode*  _upExitNode   = nullptr; ////// upper node assign to exit
        PseudoNode*  _fromWaitNode = nullptr;
        PseudoNode*  _exitNode     = nullptr;

        ////// notify Node
        PseudoNode* _upNotifyNode  = nullptr;
        PseudoNode* _notifyNode    = nullptr;

    public:

        explicit FlowBlockPipeCom(FLOW_BLOCK_TYPE fbt,
                                   Pipe pipe);
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


        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

        /**get debug*/
        void addMdLog(MdLogVal* mdLogVal) override;

        /**override simulator*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;


    };

}

#endif //KATHRYN_PIPELINECOM_H

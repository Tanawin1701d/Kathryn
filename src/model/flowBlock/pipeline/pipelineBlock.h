//
// Created by tanawin on 25/3/2567.
//

#ifndef KATHRYN_PIPELINEBLOCK_H
#define KATHRYN_PIPELINEBLOCK_H


#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#include "pipelineCom.h"
#include "pipeMng.h"

#define pipBlk                for(auto kathrynBlock = new FlowBlockPipeBase(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define pipBlkMan(recv, send) for(auto kathrynBlock = new FlowBlockPipeBase(&recv, &send); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{


    /**
     * cwhile(true){
     *      seq {
     *         pipeRcv
     *
     *         USERLOGIC
     *
     *         pipeSend
     *
     *      }
     * }
     * */

    class FlowBlockPipeBase: public FlowBlockBase, public LoopStMacro{
    protected:
        /** integrity check*/
        bool _isGetImplicitFlowBlockYet = false;


        Pipe*             _recvPipe      = nullptr;
        Pipe*             _sendPipe      = nullptr;
        FlowBlockPipeCom* _waitRecvBlock = nullptr;
        FlowBlockPipeCom* _waitSendBlock = nullptr;

        FlowBlockSeq* _implicitFlowBlock = nullptr;
        NodeWrap*     _impFbNodeWrap     = nullptr;

        PseudoNode* _upNode              = nullptr;
        PseudoNode* _jointNode           = nullptr;
        DummyNode*  _exitNode            = nullptr;


        NodeWrap* _resultNodeWrap = nullptr;


    public:

        explicit FlowBlockPipeBase();
        explicit FlowBlockPipeBase(Pipe* recvPipe, Pipe* sendPipe);
        ~FlowBlockPipeBase() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;
        NodeWrap* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/

        void buildHwComponent() override;

        void setRecvPipe(Pipe* recvPipe);
        void setSendPipe(Pipe* sendPipe);

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

#endif //KATHRYN_PIPELINEBLOCK_H

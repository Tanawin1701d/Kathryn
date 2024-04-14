//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_SEQ_H
#define KATHRYN_SEQ_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define seq for(auto kathrynBlock = new FlowBlockSeq(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn {

    class SequenceEle{
    public:
        /**node and flow block*/
        AsmNode*       _asmNode  = nullptr;

        StateNode*     _stNode   = nullptr;
        FlowBlockBase* _subBlock = nullptr;

        /**state representation*/
        NodeWrap     _nw;

        explicit SequenceEle(Node*          simpleNode);
        explicit SequenceEle(FlowBlockBase* fbBase    );
        ~SequenceEle();

        void     genNode();
        void     joinEle(SequenceEle& prevEle, NodeWrap& resultNodeWrap);

    };

    class FlowBlockSeq : public FlowBlockBase, public LoopStMacro{
    private:

        std::vector<SequenceEle*> _subSeqMetas;
        NodeWrap*                resultNodeWrap = nullptr;
        /***node for interrupt start*/
        PseudoNode* upStart   = nullptr; //// normal trigger from outsource
        PseudoNode* mainStart = nullptr; //// mainStart = upStart | interruptStart


    public:
        explicit FlowBlockSeq();
        ~FlowBlockSeq() override;
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

        /** override simulator*/

        void simStartCurCycle() override;

        void simExitCurCycle() override;

    };


}

#endif //KATHRYN_SEQ_H

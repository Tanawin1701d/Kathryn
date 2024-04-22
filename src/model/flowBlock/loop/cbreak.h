//
// Created by tanawin on 5/1/2567.
//

#ifndef KATHRYN_CBREAK_H
#define KATHRYN_CBREAK_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


#define sbreak for(auto kathrynBlock = new FlowBlockSCBreak(); kathrynBlock->doPrePostFunction(); kathrynBlock->step()){}
#define sbreakCon(expr) for(auto kathrynBlock = new FlowBlockSCBreak(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step()){}

namespace kathryn{

    class FlowBlockSCBreak : public FlowBlockBase, public LoopStMacro{
    private:
        Operable* forceExitOpr   = nullptr;
        NodeWrap* resultNodeWrap = nullptr;

        StateNode* breakNode      = nullptr;
        PseudoNode* breakCondNode = nullptr; /// incase there is condition to break node
        DummyNode* normExitNode   = nullptr;

    public:
        explicit FlowBlockSCBreak();
        explicit FlowBlockSCBreak(Operable& opr);
        ~FlowBlockSCBreak();
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
        void addMdLog(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
    };

}


#endif //KATHRYN_CBREAK_H

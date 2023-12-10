//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_SEQ_H
#define KATHRYN_SEQ_H


#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"
#include "model/FlowBlock/abstract/stateReg.h"

#define seq for(auto kathrynBlock = new FlowBlockSeq(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())


namespace kathryn {

    class SequenceEle{
    public:
        /**assignment block*/
        Node* _simpleAsm = nullptr;
        FlowBlockBase* _subBlock = nullptr;

        /**the thing that represent state*/
        ///// state for simple assignment
        StateReg* stReg = nullptr;
        Node*     stateNode = nullptr;
        ///// state for complex assignment
        NodeWrap* complexNode = nullptr;

        explicit SequenceEle(Node* simpleNode);
        explicit SequenceEle(FlowBlockBase* fbBase);
        void genHardware();

        void setDependDent(SequenceEle* predecessor);
        Operable* getStateFinishIden() const;
        std::vector<Node*> getEntranceNodes();
    };

    class FlowBlockSeq : public FlowBlockBase, public LoopStMacro{
    private:

        std::vector<SequenceEle> _subSeqMetas;
        NodeWrap* resultNodeWrap;

    public:
        explicit FlowBlockSeq();
        virtual ~FlowBlockSeq() = default;
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
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };


}

#endif //KATHRYN_SEQ_H

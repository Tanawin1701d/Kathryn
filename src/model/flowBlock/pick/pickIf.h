//
// Created by tanawin on 7/8/2024.
//

#ifndef KATHRYN_PICKIF_H
#define KATHRYN_PICKIF_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define pif(expr) for(auto kathrynBlock = new FlowBlockPickCond(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{


    class FlowBlockPickCond: public FlowBlockBase, public LoopStMacro{

    protected:

        FlowBlockBase* implicitFlowBlock = nullptr;
        Operable*      condition = nullptr;

        NodeWrap*      resultNodeWrapper = nullptr;

    public:

        explicit FlowBlockPickCond(Operable& cond);
        ~FlowBlockPickCond() override;

        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* fb) override;
        NodeWrap* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

        [[nodiscard]]
        Operable* getCondition() const{
            return condition;
        }
    };


}

#endif //PICKIF_H

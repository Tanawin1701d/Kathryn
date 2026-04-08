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

        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        void add_con_flow_block(FlowBlockBase* fb) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        [[nodiscard]]
        Operable* getCondition() const{
            return condition;
        }
    };


}

#endif //PICKIF_H

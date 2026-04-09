//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_ZELIF_H
#define KATHRYN_ZELIF_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "zifClassAsm.h"


#define zelif(expr) for(auto kathryn_block = new FlowBlockZELIF(expr); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define zelse       for(auto kathryn_block = new FlowBlockZELIF();     kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockZELIF: public FlowBlockBase, public LoopStMacro{
    private:
        Operable* curCond = nullptr;
        std::vector<ZifClassAsm*> _assignMetas;

    public :
        Operable* getPurifiedCurCond();

        explicit FlowBlockZELIF(Operable& cond);
        explicit FlowBlockZELIF();
        ~FlowBlockZELIF() override;


        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        void add_con_flow_block(FlowBlockBase* fb) override;
        void add_intr_signal(INT_TYPE type, Operable* signal) override;
        NodeWrap* sumarize_block() override;
        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        std::vector<ZifClassAsm*> getClassAssMetas();

    };

}

#endif //KATHRYN_ZELIF_H

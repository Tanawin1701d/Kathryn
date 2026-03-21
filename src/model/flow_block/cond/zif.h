//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_ZIF_H
#define KATHRYN_ZIF_H

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"
#include "zif_class_asm.h"

#define zif(expr) for(auto kathryn_block = new FlowBlockZIF(expr); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{



    class FlowBlockZIF: public FlowBlockBase,
                        public LoopStMacro{
    private:
        bool last_zelif_detected = false;
        Operable* purified_cur_cond  = nullptr;
        std::vector<Operable*> prev_falses;

        //// node will  be delete in this stage
        std::vector<ZifClassAsm*> _assignMetas;



    public:

        explicit FlowBlockZIF(Operable& cond);
        ~FlowBlockZIF() override;

        /** for controller add the local element to this sub block*/
        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        void add_con_flow_block(FlowBlockBase* fb) override;
        void add_int_signal(INT_TYPE type, Operable* signal) override;
        NodeWrap* sumarize_block() override;
        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal *md_log_val) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;
        /** extracted system*/
        std::vector<AsmNode*> extract() override;
    };



}

#endif //KATHRYN_ZIF_H

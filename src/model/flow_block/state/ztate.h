//
// Created by tanawin on 10/1/26.
//

#ifndef MODEL_FLOWBLOCK_STATE_ZTATE_H
#define MODEL_FLOWBLOCK_STATE_ZTATE_H

#include "ztate_class_asm.h"
#include "zcase.h"
#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"
#include "model/flow_block/abstract/loop_st_macro.h"

#define ztate(ident_state) for(auto kathryn_block = new FlowBlockZtate(ident_state); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockZtate: public FlowBlockBase,
                          public LoopStMacro{
    private:
        Operable& _identState;
        std::vector<int>             _caseMatchPool; ///// total match case that it have
        std::vector<ZStateClassAsm*> _assignMetas;

    public:
        explicit FlowBlockZtate(Operable& ident_state);
        ~FlowBlockZtate() override;

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

#endif //MODEL_FLOWBLOCK_STATE_ZTATE_H

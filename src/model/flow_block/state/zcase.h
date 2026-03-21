//
// Created by tanawin on 10/1/26.
//

#ifndef MODEL_FLOWBLOCK_STATE_ZCASE_H
#define MODEL_FLOWBLOCK_STATE_ZCASE_H

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"

#define zcase(case_value) for(auto kathryn_block = new FlowBlockZCase(case_value); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define zcasedef for(auto kathryn_block = new FlowBlockZCase(); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockZCase: public FlowBlockBase, public LoopStMacro{
    private:
        int _caseValue;
        std::vector<ClassAssignMeta*> _assignMetas;

    public:
        explicit FlowBlockZCase(int case_value = -1);
        ~FlowBlockZCase() override;

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

        std::vector<ClassAssignMeta*> get_class_ass_metas();
        int get_case_id() const;




    };

}

#endif //SRC_MODEL_FLOWBLOCK_STATE_ZCASE_H
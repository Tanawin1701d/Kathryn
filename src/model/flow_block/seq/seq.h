//
// Created by tanawin on 5/12/2566.
//

#ifndef MODEL_FLOWBLOCK_SEQ_SEQ_H
#define MODEL_FLOWBLOCK_SEQ_SEQ_H


#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"
#include "seq_ele.h"


#define seq for(auto kathryn_block = new FlowBlockSeq(); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn {



    class FlowBlockSeq : public FlowBlockBase, public LoopStMacro{
    private:

        std::vector<SequenceEle*> _subSeqMetas;
        NodeWrap*                result_node_wrap = nullptr;


    public:
        explicit FlowBlockSeq();
        ~FlowBlockSeq() override;
        /** for controller add the local element to this sub block*/
        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal* md_log_val) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;
    };


}

#endif //MODEL_FLOWBLOCK_SEQ_SEQ_H

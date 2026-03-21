//
// Created by tanawin on 14/2/26.
//

#ifndef MODEL_FLOWBLOCK_LOOP_LOOP_H
#define MODEL_FLOWBLOCK_LOOP_LOOP_H

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/cnt_node.h"


#define cloop(kathryn_loop_name, loop_number) \
    for(auto kathryn_block = new FlowBlockLoop(loop_number); kathryn_block->do_pre_post_function(); kathryn_block->step()) \
        for (Operable& kathryn_loop_name = kathryn_block->get_loop_id(); !kathryn_block->is_gotten_loop_var(); kathryn_block->set_get_loop_var())

namespace kathryn{

    class FlowBlockLoop: public FlowBlockBase, public LoopStMacro{
    protected:
        int _loopCount = 0;
        bool get_loop_var = false;
        //////// block
        FlowBlockBase* _implicitFlowBlock = nullptr;
        bool           _isGetFlowBlockYet = false;
        //////// nodes
        NodeWrap*      _resultNodeWrapper = nullptr;
        NodeWrap*      _subBlockNodeWrap  = nullptr;
        PseudoNode*    _entNode           = nullptr;
        PseudoNode*    _loopNode          = nullptr;
        CounterNode*    _cntNode          = nullptr;
        PseudoNode*    _exitNode          = nullptr;
        //////// user wire
        expression*    _loopId            = nullptr;


        //// it is wrap is as same as result but it is used for loop assignment



    public:

        explicit FlowBlockLoop(int loop_count);
        ~FlowBlockLoop() override;

        void build_hw_component() override;

        /** for controller add the local element to this sub block*/
        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        ///////////void build_hw_component() override;

        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        void add_md_log(MdLogVal* md_log_val) override;

        Operable& get_loop_id(){return *_loopId;}

        void set_get_loop_var(){ get_loop_var = true; }
        bool is_gotten_loop_var(){return get_loop_var;}
        
    };

}

#endif //MODEL_FLOWBLOCK_LOOP_LOOP_H
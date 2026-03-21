//
// Created by tanawin on 7/8/2024.
//

#include "pick_if.h"

#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockPickCond::FlowBlockPickCond(Operable& cond):
    FlowBlockBase(PICK_WHEN,
        {
               {FLOW_ST_BASE_STACK},
               FLOW_JO_SUB_FLOW,
               true
        }
    ),
    condition(&cond){}

    FlowBlockPickCond::~FlowBlockPickCond(){
        //// delete result_node_wrapper; it is derive from inside block do not delete
    }

    void FlowBlockPickCond::add_element_in_flow_block(Node* node){
        assert(false);
    }

    void FlowBlockPickCond::add_sub_flow_block(FlowBlockBase* sub_block){
        assert(sub_block != nullptr);
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    void FlowBlockPickCond::add_con_flow_block(FlowBlockBase* con_block){
        assert(false);
    }

    NodeWrap* FlowBlockPickCond::sumarize_block(){
        assert(result_node_wrapper != nullptr);
        return result_node_wrapper;
    }

    void FlowBlockPickCond::on_attach_block(){
        ctrl->on_attach_flowBlock(this);
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicit_flow_block = sb;
        sb->on_attach_block();
    }

    void FlowBlockPickCond::on_detach_block(){
        assert(implicit_flow_block != nullptr);
        implicit_flow_block->on_detach_block();
        /////// no need to set lazy delete
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPickCond::build_hw_component(){
        assert(_conBlocks.empty());
        assert(!_subBlocks.empty());
        mf_assert(!is_there_int_start(), "start interrupt can start in elif block but can start in if block or block inside elif block");
        ///mf_assert(!is_there_int_rst(), "start interrupt can reset in elif block but can start in if block or block inside elif block");
        result_node_wrapper = _subBlocks[0]->sumarize_block();

    }

    void FlowBlockPickCond::do_pre_function(){
        on_attach_block();
    }

    void FlowBlockPickCond::do_post_function(){
        on_detach_block();
    }


}

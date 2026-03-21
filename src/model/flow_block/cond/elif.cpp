//
// Created by tanawin on 8/12/2566.
//

#include "elif.h"
#include "model/controller/controller.h"
#include "model/flow_block/cond/if.h"


namespace kathryn{

    /** constructor*/
    FlowBlockElif::FlowBlockElif(Operable &cond):
    FlowBlockBase(CSELIF,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_CON_FLOW,
                          false
                  }),
    _cond(&cond){}

    FlowBlockElif::FlowBlockElif():
    FlowBlockBase(CSELSE,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_CON_FLOW,
                          false
                  }) {}

    FlowBlockElif::~FlowBlockElif(){
        //// delete result_node_wrapper; it is derive from inside block do not delete

    };

    void FlowBlockElif::add_element_in_flow_block(Node* node) {
        assert(false); //// due to implicit sublock declaration
    }

    void FlowBlockElif::add_sub_flow_block(FlowBlockBase* sub_block) {
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    NodeWrap *FlowBlockElif::sumarize_block() {
        assert(result_node_wrapper != nullptr);
        return result_node_wrapper;
    }

    void FlowBlockElif::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicit_sub_block = sb;
        sb->on_attach_block();
    }

    void FlowBlockElif::on_detach_block() {
        implicit_sub_block->on_detach_block();
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockElif::build_hw_component() {
        assert(_conBlocks.empty());
        assert(!_subBlocks.empty());
        mf_assert(!is_there_int_start(), "start interrupt can start in elif block but can start in if block or block inside elif block");
        ///mf_assert(!is_there_int_rst(), "start interrupt can reset in elif block but can start in if block or block inside elif block");
        result_node_wrapper = _subBlocks[0]->sumarize_block();
    }

    void FlowBlockElif::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockElif::do_post_function() {
        on_detach_block();
    }

    std::string FlowBlockElif::get_md_describe() {
        std::string ret;
        ret += "[ " + FlowBlockBase::get_md_ident_val() + " ]\n";
        ret += "[elif implicit_flow_block]" + implicit_sub_block->get_md_describe() + "\n";
        return ret;
    }

    void FlowBlockElif::add_md_log(MdLogVal *md_log_val){
        md_log_val->add_val("[ Elif " + FlowBlockBase::get_md_ident_val() + " ]");
        add_md_log_recur(md_log_val);
    }


}
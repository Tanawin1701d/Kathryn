//
// Created by tanawin on 8/12/2566.
//

#include "elif.h"
#include "model/controller/controller.h"
#include "model/flowBlock/cond/if.h"


namespace kathryn{
    /// constructor
    FlowBlockElif::FlowBlockElif(Operable& cond) :
        FlowBlockBase(CSELIF,
                      {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_CON_FLOW,
                          false
                      }),
        _cond(&cond){
    }

    FlowBlockElif::FlowBlockElif() :
        FlowBlockBase(CSELSE,
                      {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_CON_FLOW,
                          false
                      }){
    }

    FlowBlockElif::~FlowBlockElif(){
        /// delete resultNodeWrapper; it is derive from inside block do not delete
    };

    void FlowBlockElif::add_basic_node(Node* node){
        assert(false); /// due to implicit sublock declaration
    }

    void FlowBlockElif::add_sub_flow_block(FlowBlockBase* subBlock){
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    NodeWrap* FlowBlockElif::sumarize_block(){
        assert(_result_node_wrapper != nullptr);
        return _result_node_wrapper;
    }

    void FlowBlockElif::on_attach_block(){
        _ctrl->on_attach_flowBlock(this);
        auto sub_block = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        _implicit_sub_block = sub_block;
        sub_block->on_attach_block();
    }

    void FlowBlockElif::on_detach_block(){
        _implicit_sub_block->on_detach_block();
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockElif::build_hw_component(){
        assert(_con_blocks.empty());
        assert(!_sub_blocks.empty());
        mf_assert(!is_there_intr_start(),
                  "start interrupt can start in elif block but can start in if block or block inside elif block");
        /// mfAssert(!isThereIntRst(), "start interrupt can reset in elif block but can start in if block or block inside elif block");
        _result_node_wrapper = _sub_blocks[0]->sumarize_block();
    }

    void FlowBlockElif::do_pre_function(){
        on_attach_block();
    }

    void FlowBlockElif::do_post_function(){
        on_detach_block();
    }

    std::string FlowBlockElif::get_md_describe(){
        std::string ret;
        ret += "[ " + FlowBlockBase::get_md_ident_val() + " ]\n";
        ret += "[elif implicitFlowBlock]" + _implicit_sub_block->get_md_describe() + "\n";
        return ret;
    }

    void FlowBlockElif::add_md_log(MdLogVal* md_log_val){
        md_log_val->addVal("[ Elif " + FlowBlockBase::get_md_ident_val() + " ]");
        add_md_log_recur(md_log_val);
    }
}

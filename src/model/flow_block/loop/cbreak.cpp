//
// Created by tanawin on 5/1/2567.
//

#include "cbreak.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockSCBreak::FlowBlockSCBreak():
    FlowBlockBase(EXITWHILE,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_SUB_FLOW,
                          true
                  }),
    LoopStMacro() {}

    FlowBlockSCBreak::FlowBlockSCBreak(Operable& opr1):
            FlowBlockBase(EXITWHILE,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }),
            LoopStMacro(),
            force_exit_opr(&opr1){}

    FlowBlockSCBreak::~FlowBlockSCBreak(){
        delete result_node_wrap;
        delete break_node;
        delete break_cond_node;
        delete norm_exit_node;
    }

    void FlowBlockSCBreak::add_element_in_flow_block(Node *node) {
        assert(false);
    }

    void FlowBlockSCBreak::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(false);
    }

    NodeWrap* FlowBlockSCBreak::sumarize_block() {
        assert(result_node_wrap != nullptr);
        return result_node_wrap;
    }

    void FlowBlockSCBreak::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockSCBreak::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockSCBreak::build_hw_component() {

        assert(_conBlocks.empty());
        /**build internal node*/
        break_node = new StateNode(get_clock_mode());
        add_sys_node(break_node);
        fill_int_reset_to_node_if_there(break_node);

        norm_exit_node = new DummyNode(&make_opr_val("cbreak_dum", 1, 0));
        add_sys_node(norm_exit_node);
        /////////norm_exit_node->add_depend_node(break_node);
        norm_exit_node->assign();

        /**build result_node_wrap*/
        result_node_wrap = new NodeWrap();
        result_node_wrap->add_entrace_node(break_node);
        if (force_exit_opr == nullptr){
            result_node_wrap->add_force_exit_node(break_node);
        }else{
            break_cond_node = new PseudoNode(1, BITWISE_AND);
            break_cond_node->add_depend_node(break_node, force_exit_opr);
            break_cond_node->assign();
            add_sys_node(break_cond_node);
            result_node_wrap->add_force_exit_node(break_cond_node);
        }
        result_node_wrap->add_exit_node(norm_exit_node);

    }

    std::string FlowBlockSCBreak::get_md_describe(){
        std::string ret;
        ret += "[break_node is]"+ (break_node != nullptr ? break_node->get_md_describe() : "") + "\n";
        return ret;
    }

    void FlowBlockSCBreak::add_md_log(MdLogVal *md_log_val) {
        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        md_log_val->add_val("break_node is " +
                             (break_node != nullptr ? break_node->get_md_describe() : ""));
    }

    void FlowBlockSCBreak::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockSCBreak::do_post_function() {
        on_detach_block();
    }
}
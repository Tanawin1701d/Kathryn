//
// Created by tanawin on 22/4/2567.
//

#include "do_while_base.h"
#include "model/controller/controller.h"


namespace kathryn {

    FlowBlockDowhile::FlowBlockDowhile(Operable &cond_expr,
                                       FLOW_BLOCK_TYPE fbt) :
            _condExpr(&cond_expr),
            _purifiedCondExpr(purify_condition(&cond_expr)),
            FlowBlockBase(fbt,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }) {
        assert(_purifiedCondExpr != nullptr);
    }

    FlowBlockDowhile::~FlowBlockDowhile() {
        delete result_node_wrapper;
        delete exit_node;
    }


    void FlowBlockDowhile::build_hw_component() {
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        sub_block_node_wrap = _subBlocks[0]->sumarize_block();
        assert(sub_block_node_wrap != nullptr);

        /***sub block depend lower deck is assume condition or*/
        sub_block_node_wrap->add_depend_node_to_all_node(sub_block_node_wrap->get_exit_node(),
                                                 sub_block_node_wrap->is_there_force_exit_node()
                                                 ? &((*_purifiedCondExpr) & (~(*sub_block_node_wrap->get_force_exit_node_ptr()->get_exit_opr_ptr())))
                                                 : _purifiedCondExpr);
        if (is_there_int_start()) {
            sub_block_node_wrap->add_depend_node_to_all_node(int_nodes[INT_START], nullptr);
        }
        /** exit node*/
        //////// no need reset signal
        exit_node = new PseudoNode(1, BITWISE_OR);
        exit_node->add_depend_node(sub_block_node_wrap->get_exit_node(), &(!(*_purifiedCondExpr)));
        if (sub_block_node_wrap->is_there_force_exit_node()){
            exit_node->add_depend_node(sub_block_node_wrap->get_force_exit_node_ptr(), nullptr);
        }
        exit_node->assign();
        add_sys_node(exit_node);
        ////////////////////////////////////////////////////////////////////
        result_node_wrapper = new NodeWrap();
        result_node_wrapper->transfer_ent_node_from(sub_block_node_wrap);
        result_node_wrapper->add_exit_node(exit_node);

    }


    void FlowBlockDowhile::add_element_in_flow_block(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockDowhile::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(sub_block != nullptr);
        assert(!is_get_flow_block_yet);
        is_get_flow_block_yet = true;
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    NodeWrap *FlowBlockDowhile::sumarize_block() {
        assert(result_node_wrapper != nullptr);
        return result_node_wrapper;
    }

    void FlowBlockDowhile::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicit_flow_block = sb;
        sb->on_attach_block();
    }

    void FlowBlockDowhile::on_detach_block() {
        assert(implicit_flow_block != nullptr);
        implicit_flow_block->on_detach_block();
        assert(is_get_flow_block_yet);
        ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockDowhile::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockDowhile::do_post_function() {
        on_detach_block();
    }

    void FlowBlockDowhile::add_md_log(MdLogVal *md_log_val) {

        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        md_log_val->add_val("exit_node " + exit_node->get_md_ident_val() + " " + exit_node->get_md_describe());
        md_log_val->add_val("result_node_wrap is" +
                         result_node_wrapper->get_md_ident_val() + " " + result_node_wrapper->get_md_describe());

        auto sub_log = md_log_val->make_new_sub_val();
        implicit_flow_block->add_md_log(sub_log);

    }

}
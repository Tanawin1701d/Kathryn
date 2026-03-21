//
// Created by tanawin on 6/12/2566.
//

#include "while_base.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockWhile::FlowBlockWhile(Operable& cond_expr,
                                   FLOW_BLOCK_TYPE fbt):
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

    FlowBlockWhile::FlowBlockWhile(bool fall_true,
                                   FLOW_BLOCK_TYPE fbt):
            _fallTrue(fall_true),
            FlowBlockBase(fbt,{
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }) {
        assert(fall_true);
        assert(_purifiedCondExpr == nullptr);
    }



    FlowBlockWhile::~FlowBlockWhile() {
        delete result_node_wrapper;
        delete condition_node;
        delete exit_dummy;
        delete exit_node;
    }


    void FlowBlockWhile::build_hw_component(){
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        sub_block_node_wrap = _subBlocks[0]->sumarize_block();
        assert(sub_block_node_wrap != nullptr);


        //** initialize node*/
        if (get_flow_type() == CWHILE){
            condition_node = new PseudoNode(1, BITWISE_OR);
            condition_node->set_internal_ident("c_con_node" + std::to_string(get_global_id()));
        }else{////// SWHILE
            condition_node = new StateNode(get_clock_mode());
            condition_node->set_internal_ident("s_con_node" + std::to_string(get_global_id()));
            fill_int_reset_to_node_if_there(condition_node);
            fill_hold_to_node_if_there(condition_node);
        }
        add_sys_node(condition_node);

        exit_node          = new PseudoNode(1, BITWISE_OR);
        add_sys_node(exit_node);
        result_node_wrapper = new NodeWrap();
        ////////////////////////////////////////////////////////////////////

        /** do sub block dep init*/
        sub_block_node_wrap->add_depend_node_to_all_node(condition_node, _purifiedCondExpr);
        sub_block_node_wrap->assign_all_node();

        /**do condition node Dep*/
            //// codition trigger from outside willbe trigger in upper node
        condition_node->add_depend_node(sub_block_node_wrap->get_exit_node(),
                                     sub_block_node_wrap->is_there_force_exit_node()?
                                        ( &(~(*sub_block_node_wrap->get_force_exit_node_ptr()->get_exit_opr_ptr())) ):
                                        nullptr
                                     );
        if(is_there_int_start()){
            condition_node->add_depend_node(int_nodes[INT_START], nullptr);
        }
        /**do exit NOde Dep*/
        if (!_fallTrue) {
            exit_node->add_depend_node(condition_node, &(!(*_purifiedCondExpr)) );
        }
        if (sub_block_node_wrap->is_there_force_exit_node()){
            exit_node->add_depend_node(sub_block_node_wrap->get_force_exit_node_ptr(), nullptr);
        }

        if (_fallTrue && (!sub_block_node_wrap->is_there_force_exit_node())){
            ///////// incase there is no exit source we warning user that there is infinite loop
            /////////// TODO warning
            exit_dummy = new DummyNode(&make_opr_val("exit_dummy",1, 0));
            add_sys_node(exit_dummy);
            exit_node->add_depend_node(exit_dummy, nullptr);
        }

        exit_node->assign();


        result_node_wrapper->add_entrace_node(condition_node);
        result_node_wrapper->add_exit_node(exit_node);

    }



    void FlowBlockWhile::add_element_in_flow_block(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockWhile::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(sub_block != nullptr);
        assert(!is_get_flow_block_yet);
        is_get_flow_block_yet = true;
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    NodeWrap* FlowBlockWhile::sumarize_block() {
        assert(result_node_wrapper != nullptr);
        return result_node_wrapper;
    }

    void FlowBlockWhile::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicit_flow_block = sb;
        sb->on_attach_block();
    }

    void FlowBlockWhile::on_detach_block() {
        assert(implicit_flow_block != nullptr);
        implicit_flow_block->on_detach_block();
        assert(is_get_flow_block_yet);
        ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockWhile::do_pre_function() {
        on_attach_block();
    }
    void FlowBlockWhile::do_post_function() {
        on_detach_block();
    }

    void FlowBlockWhile::add_md_log(MdLogVal* md_log_val){

        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        md_log_val->add_val("con_node " + condition_node->get_md_ident_val() + " " + condition_node->get_md_describe());
        md_log_val->add_val("exit_node " + exit_node->get_md_ident_val() + " " + exit_node->get_md_describe());
        md_log_val->add_val("result_node_wrap is" +
                         result_node_wrapper->get_md_ident_val() + " " + result_node_wrapper->get_md_describe());

        auto sub_log = md_log_val->make_new_sub_val();
        implicit_flow_block->add_md_log(sub_log);

    }

}
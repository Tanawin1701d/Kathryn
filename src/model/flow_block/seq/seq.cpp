//
// Created by tanawin on 5/12/2566.
//

#include "seq.h"
#include "model/controller/controller.h"

namespace kathryn{

    /**
     *
     *
     * sequence flow
     *
     *
     * */

    FlowBlockSeq::FlowBlockSeq(): FlowBlockBase(SEQUENTIAL,
                                                {
                                                        {FLOW_ST_BASE_STACK,
                                                         FLOW_ST_PATTERN_STACK},
                                                        FLOW_JO_SUB_FLOW,
                                                        true
                                                }),
                                  result_node_wrap(nullptr) {}

    FlowBlockSeq::~FlowBlockSeq(){
        delete result_node_wrap;
        for (auto seq_ele : _subSeqMetas){
            delete seq_ele;
        }
    }

    void FlowBlockSeq::add_element_in_flow_block(Node* node) {
        assert(node != nullptr);
        _subSeqMetas.push_back(new SequenceEleBasic(node));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::add_element_in_flow_block(node);
    }

    void FlowBlockSeq::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(sub_block != nullptr);
        _subSeqMetas.emplace_back(new SequenceEleFlowBlock(sub_block));
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    NodeWrap* FlowBlockSeq::sumarize_block() {
        assert(result_node_wrap != nullptr);
        return result_node_wrap;
    }

    void FlowBlockSeq::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockSeq::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockSeq::build_hw_component() {
        mf_assert(!_subSeqMetas.empty(), "seq_block has no assignment");
        assert(_conBlocks.empty());
        NodeWrapCycleDet cycle_det;
        /** generate hardware*/
        int idx = 0;
        for (auto& seq_meta: _subSeqMetas) {
            seq_meta->set_int_reset(int_nodes[INT_RESET]); //// set interrupt reset must be set before gennode
            seq_meta->set_hold_node(hold_node);
            seq_meta->gen_node(get_clock_mode());
            seq_meta->set_ident_state_id(get_global_id(),idx++);
            seq_meta->add_to_cycle_det(cycle_det);
            seq_meta->add_to_system_nodes(_sysNodes);
        }
        /** generate force_exit Node*/
            /***check are_there force_exit_node*/
        std::vector<NodeWrap*> all_nw;
        for(auto seq_ele: _subSeqMetas){
            if (seq_ele->is_node_wrap()){
                all_nw.push_back(seq_ele->get_node_wrap());
            }
        }
        gen_sum_force_exit_node(all_nw);
        /** connect depend node chain*/
        for (size_t idx = 0; (idx+1) < _subSeqMetas.size(); idx++){
            auto lhs_node_wrapper = _subSeqMetas[idx];
            auto rhs_node_wrapper = _subSeqMetas[idx+1];
            rhs_node_wrapper->assign_depend_dent(lhs_node_wrapper);
        }
        if (is_there_int_start()){
            _subSeqMetas[0]->assign_int_start(int_nodes[INT_START]);
        }

        /** build new result NodeWrap*/
        result_node_wrap = new NodeWrap();
        result_node_wrap->add_entrace_nodes((*_subSeqMetas.begin())->get_entrance_nodes());
        result_node_wrap->add_exit_node((*_subSeqMetas.rbegin())->get_state_finish_iden());
        result_node_wrap->set_cycle_used(cycle_det.get_cycle_vertical());
        if (_areThereForceExit)
            result_node_wrap->add_force_exit_node(_forceExitNode);

    }



    std::string FlowBlockSeq::get_md_describe() {

        std::string ret;
        int ele_cnt = 0;
        ret += "[ " + FlowBlockBase::get_md_ident_val() + " ]\n";
        for (auto seq_ele: _subSeqMetas){
            ret += seq_ele->get_describe() + "\n";
            ele_cnt++;
        }
        ret += get_md_describe_recur();
        return ret;

    }

    void FlowBlockSeq::add_md_log(MdLogVal *md_log_val) {

        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        for (auto seq_ele: _subSeqMetas){
            md_log_val->add_val(seq_ele->get_describe());
        }
        if (result_node_wrap->is_there_force_exit_node()){
            md_log_val->add_val("force_exit is " + result_node_wrap->get_force_exit_node_ptr()->get_md_ident_val() +
                                                   "  " +
                                                   result_node_wrap->get_force_exit_node_ptr()->get_md_describe());
        }

        add_md_log_recur(md_log_val);

    }

    void FlowBlockSeq::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockSeq::do_post_function() {
        on_detach_block();
    }

}

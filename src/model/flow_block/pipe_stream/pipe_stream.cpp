//
// Created by tanawin on 7/2/26.
//

#include "pipe_stream.h"
#include "model/controller/controller.h"

namespace kathryn{

    /**
     * pipestream flow
     */
    FlowBlockPipeStream::FlowBlockPipeStream():
        FlowBlockBase(PIPE_STREAM,
                      {
                        {FLOW_ST_BASE_STACK},
                        FLOW_JO_SUB_FLOW,
                        true
                      }){}

    FlowBlockPipeStream::~FlowBlockPipeStream(){
        for (auto pip_stream_ele: _subStreamEles){
            delete pip_stream_ele;
        }
        delete _resultNodeWrap;
        delete _dummyNode;
    }

    void FlowBlockPipeStream::add_element_in_flow_block(Node* node){
        assert(node != nullptr);
        auto seq_ele = new SequenceEleBasic(node);
        _subStreamEles.push_back(new StreamEle(seq_ele));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::add_element_in_flow_block(node);
    }

    void FlowBlockPipeStream::add_sub_flow_block(FlowBlockBase* sub_block){
        assert(sub_block != nullptr);
        auto seq_ele = new SequenceEleFlowBlock(sub_block);
        _subStreamEles.emplace_back(new StreamEle(seq_ele));
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    NodeWrap* FlowBlockPipeStream::sumarize_block(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockPipeStream::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPipeStream::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeStream::build_hw_component() {
        mf_assert(!_subStreamEles.empty(), "pipestream flow must have at least one element");
        assert(_conBlocks.empty());
        /**generate hardware*/
        std::vector<Node*> all_start_nodes;
        int idx = 0;
        for (auto& stream_meta: _subStreamEles){
            ///////// initialize hardware
            stream_meta->set_int_reset(int_nodes[INT_RESET]); //// set interrupt reset must be set before gennode
            stream_meta->set_hold_node(hold_node);
            stream_meta->gen_node(get_clock_mode());
            stream_meta->set_ident_state_id(get_global_id(),idx);
            stream_meta->add_to_system_nodes(_sysNodes);
            idx++;
        }

        for (idx = 0; idx < _subStreamEles.size(); idx++){
            ////////// connect the dependency
            StreamEle* stream_meta = _subStreamEles[idx];
            StreamEle* prev_ele = (idx == 0)
                                  ? nullptr: _subStreamEles[idx-1];
            StreamEle* next_ele = (idx == (_subStreamEles.size()-1))
                                  ? nullptr: _subStreamEles[idx+1];
            _subStreamEles[idx]->add_sync_dependency(prev_ele, next_ele);
            if (is_there_int_start()){
                _subStreamEles[idx]->assign_int_start(int_nodes[INT_START]);
            }
            all_start_nodes.push_back(stream_meta->get_entrance_node_ptr());
        }

        make_val(dummy_pip_stream_val, 1,0);
        _dummyNode = new DummyNode(&dummy_pip_stream_val);

        /**result Node wrap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->add_entrace_nodes(all_start_nodes);
        _resultNodeWrap->add_exit_node(_dummyNode);

    }

    std::string FlowBlockPipeStream::get_md_describe() {

        // std::string ret;
        // int ele_cnt = 0;
        // ret += "[ " + FlowBlockBase::get_md_ident_val() + " ]\n";
        // for (auto seq_ele: _subSeqMetas){
        //     ret += seq_ele->get_describe() + "\n";
        //     ele_cnt++;
        // }
        // ret += get_md_describe_recur();
        // return ret;
        return "";

    }

    void FlowBlockPipeStream::add_md_log(MdLogVal *md_log_val) {

        // md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        // for (auto seq_ele: _subSeqMetas){
        //     md_log_val->add_val(seq_ele->get_describe());
        // }
        // if (result_node_wrap->is_there_force_exit_node()){
        //     md_log_val->add_val("force_exit is " + result_node_wrap->get_force_exit_node_ptr()->get_md_ident_val() +
        //                                            "  " +
        //                                            result_node_wrap->get_force_exit_node_ptr()->get_md_describe());
        // }
        //
        // add_md_log_recur(md_log_val);


    }

    void FlowBlockPipeStream::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockPipeStream::do_post_function() {
        on_detach_block();
    }


}

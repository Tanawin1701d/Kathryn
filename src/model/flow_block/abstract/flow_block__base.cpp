//
// Created by tanawin on 2/12/2566.
//

#include "flow_block__base.h"

#include "utility"
#include "model/controller/controller.h"
#include "model/flow_block/seq/seq.h"
#include "model/flow_block/par/par.h"

namespace kathryn{

    int next_fb_idx = 0;


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type, FB_CTRL_COM_META fb_ctrl_com_meta):
            FlowIdentifiable(FBT_to_string(type)),
            /** flow element*/
            _type(type),
            ctrl(get_controller_ptr()),
            lazy_deleted_required(false),
            _fbId(next_fb_idx++),
            /** controller communication policy*/
            _fbCtrlComMeta(std::move(fb_ctrl_com_meta)),
            /** exit management*/
            _areThereForceExit(false),
            _forceExitNode(nullptr),
            _flowSimEngine(new FlowBaseSimEngine(this))
    {
                /**initialize interrupt node*/
                for (int int_type = 0; int_type < INT_CNT; int_type++){
                    int_nodes[int_type] = nullptr;
                }
                ///set_clock_mode(GET_CLOCK_MODE()); ///// get clock mode from master0
                set_clock_mode(CM_POSEDGE); ///// get clock mode from master0
    }

    FlowBlockBase::~FlowBlockBase(){
        for (auto basic_node : _basicNodes){
            delete basic_node;
        }
        for (auto sub_fb: _subBlocks){
            delete sub_fb;
        }
        for (auto con_fb: _conBlocks){
            delete con_fb;
        }
        for (auto abandon_fb:_abandonedBlocks){
            delete abandon_fb;
        }
        for (auto int_node: int_nodes){
            delete int_node;
        }
        delete hold_node;

        delete _forceExitNode;
        delete _flowSimEngine;
        /////// it is safe to delete nullptr
    }

    FlowBlockBase* FlowBlockBase::gen_implicit_sub_blk(FLOW_BLOCK_TYPE default_type) {
        /** determine next flow block*/
        FLOW_BLOCK_TYPE next_fb_type = ctrl->get_top_pattern_flow_block_type();
        if (next_fb_type == DUMMY_BLOCK){
            next_fb_type = default_type;
        }
        /** create subblock*/
        if ( (next_fb_type == PARALLEL_NO_SYN) || (next_fb_type == PARALLEL_AUTO_SYNC)){
            return new FlowBlockParAuto();
        }else if (next_fb_type == SEQUENTIAL){
            return new FlowBlockSeq();
        }else{
            assert(false); /** can't determine flow type*/
        }
    }

    void FlowBlockBase::gen_sum_force_exit_node(std::vector<NodeWrap *> &nws) {
        /** check that there is force exit node*/
        for (auto nw : nws){
            _areThereForceExit |= (nw->get_force_exit_node_ptr() != nullptr);
        }

        /** build pseudo node*/
        if (_areThereForceExit){
            _forceExitNode = new PseudoNode(1, BITWISE_OR);
            for (auto nw : nws){
                if (nw->get_force_exit_node_ptr() != nullptr){
                    _forceExitNode->add_depend_node(nw->get_force_exit_node_ptr(), nullptr);
                }
            }
            _forceExitNode->assign();
        }
    }

    void FlowBlockBase::fill_int_rst_signal_to_child(){
        for (auto sub_block_ptr: _subBlocks){
            assert(sub_block_ptr != nullptr);
            for (auto signal: int_signals[INT_RESET]){
                sub_block_ptr->add_int_signal(INT_RESET, signal);
            }

        }

        for (auto con_block_ptr: _conBlocks){
            assert(con_block_ptr != nullptr);
            for (auto signal: int_signals[INT_RESET]){
                con_block_ptr->add_int_signal(INT_RESET, signal);
            }
        }
    }

    void FlowBlockBase::gen_int_node(){

        for (int int_type = 0; int_type < INT_CNT; int_type++){
            if (int_signals[int_type].empty()){
                continue;
            }
            /* init node **/
            int_nodes[int_type] = new OprNode(int_signals[int_type][0]);

            for (int sig_id = 1; sig_id < int_signals[int_type].size(); sig_id++){
                int_nodes[int_type]->add_logic(
                    int_nodes[int_type]->_value,
                    int_signals[int_type][sig_id],
                    BITWISE_OR
                );
            }
        }
    }

    Operable* FlowBlockBase::gen_int_sum_signal(bool is_and_cond, INT_TYPE intr_type){

        std::vector<Operable*>& target_signals = int_signals[intr_type];

        if (target_signals.empty()){
            return nullptr;
        }
        Operable* result_signal = target_signals[0];
        for (int idx = 1; idx < static_cast<int>(target_signals.size()); idx++){
            mf_assert(target_signals[idx]->get_operable_slice().get_size() == 1, "interrupt signal size cannot more than one");

            result_signal = is_and_cond ? &((*result_signal) & (*target_signals[idx]))
                                     : &((*result_signal) | (*target_signals[idx]));
        }
        return result_signal;

    }

    bool FlowBlockBase::is_there_int_start(){
        return int_nodes[INT_START] != nullptr;
    }

    bool FlowBlockBase::is_there_int_rst(){
        return int_nodes[INT_RESET] != nullptr;
    }

    void FlowBlockBase::fill_hold_signal_to_child(){
        for (auto sub_block_ptr: _subBlocks){
            assert(sub_block_ptr != nullptr);
            for (auto signal: hold_signals){
                sub_block_ptr->add_hold_signal(signal);
            }
        }

        for (auto con_block_ptr: _conBlocks){
            assert(con_block_ptr != nullptr);
            for (auto signal: hold_signals){
                con_block_ptr->add_hold_signal(signal);
            }
        }
    }

    void FlowBlockBase::gen_hold_node(){

        if (hold_signals.empty()){
            return;
        }
        hold_node = new OprNode(hold_signals[0]);

        for (int sig_id = 1; sig_id < static_cast<int>(hold_signals.size()); sig_id++){
            hold_node->add_logic(
                hold_node->_value,
                hold_signals[sig_id],
                BITWISE_OR
            );
        }

    }

    bool FlowBlockBase::is_there_hold(){
        return hold_node  != nullptr;
    }

    void FlowBlockBase::set_clock_mode(CLOCK_MODE mode){
        clk_mode = mode;
        mf_assert((get_clock_mode() == CM_POSEDGE) ||
                      (get_clock_mode() == CM_NEGEDGE)
        , "clock mode in flow must be only CM_POSEDGE or CM_NEGEDGE");
    }

    void FlowBlockBase::build_hw_master(){

        /** override the asm_node     Please remind that it must be done before build_sub_hw_component
         * because sub component such as zif zelse block can put more asm_node(sub block must think by their own)*/

        /** pass the Int reset signal and holding signal to child block*/
        /** dont fill interrupt start signal because this block will start it*/
        fill_int_rst_signal_to_child();
        fill_hold_signal_to_child();

        /** build the sub block first*/
        build_sub_hw_component();

        /** we so sure now that all sub  Block is ready*/
        /** start build the node for int reset start and hold signal*/
        gen_int_node();
        gen_hold_node();


        build_hw_component();
    }

    void FlowBlockBase::build_sub_hw_component(){

        for (auto sub_block_ptr: _subBlocks){
            assert(sub_block_ptr != nullptr);
            sub_block_ptr->build_hw_master();
        }

        for (auto con_block_ptr: _conBlocks){
            assert(con_block_ptr != nullptr);
            con_block_ptr->build_hw_master();
        }

    }

    /**
     * condition such as cif sif celif may receiver condition that has
     * more than 1 in  bitsize, we need to convert it to one in bitsize
     ***/

    Operable* FlowBlockBase::purify_condition(Operable* raw_opr){

            assert(raw_opr != nullptr);
            Slice raw_sl = raw_opr->get_operable_slice();
            assert(raw_sl.get_size() != 0);

            if (raw_sl.get_size() == 1){
                return raw_opr;
            }
            make_val(auto_cmp_cond_val, raw_sl.get_size(), 0);
            return &((*raw_opr) > auto_cmp_cond_val);

    }

    FlowBlockBase* FlowBlockBase::scan_master_join_sub_block(){
        //////// the master join block must have only 1 sub block
        FlowBlockBase* result_fb = nullptr;
        for (FlowBlockBase* sub_fb: _subBlocks){
            mf_assert(sub_fb != nullptr, "sub_block cannot be nullptr");
            if (sub_fb->is_join_master()){
                mf_assert(result_fb == nullptr, "duplicated master join block");
                result_fb = sub_fb;
            }
        }
        return result_fb;
    }

    std::vector<FlowBlockBase::sort_ele> FlowBlockBase::sort_sub_and_con_fb_in_order() {

        std::vector<sort_ele> pool_ele;
        /***pool sub block in to the array*/
        for(int i = 0; i < _subBlocks.size(); i++){
            pool_ele.push_back({_subBlocks[i], _subBlocksOrder[i]});
        }
        for(int i = 0; i < _conBlocks.size(); i++){
            pool_ele.push_back({_conBlocks[i], _conBlocksOrder[i]});
        }
        /**sort array*/
        std::sort(pool_ele.begin(), pool_ele.end());

        return pool_ele;
    }

    void FlowBlockBase::override_clock_mode_in_all_asm_nodes(){

        for (Node* node: _basicNodes){
            assert(node != nullptr);
            assert(node->get_node_type() == ASM_NODE);
            ((AsmNode*)node)->override_clock_mode(get_clock_mode());
        }

    }

}
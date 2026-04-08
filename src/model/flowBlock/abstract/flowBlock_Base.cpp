//
// Created by tanawin on 2/12/2566.
//

#include "flowBlock_Base.h"

#include <utility>
#include "model/controller/controller.h"
#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/par/par.h"

namespace kathryn{

    int nextFbIdx = 0;


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type, FB_CTRL_COM_META fb_ctrl_com_meta):
            FlowIdentifiable(FBT_to_string(type)),
            /** flow element*/
            _type(type),
            _ctrl(get_controller_ptr()),
            _lazy_deleted_required(false),
            _fb_id(nextFbIdx++),
            /** controller communication policy*/
            _fb_ctrl_com_meta(std::move(fb_ctrl_com_meta)),
            /** exit management*/
            _are_there_force_exit(false),
            _force_exit_node(nullptr),
            _flow_sim_engine(new FlowBaseSimEngine(this))
    {
                /**initialize interrupt node*/
                for (int int_type = 0; int_type < INT_CNT; int_type++){
                    _int_nodes[int_type] = nullptr;
                }
                ///setClockMode(GET_CLOCK_MODE()); ///// get clock mode from master0
                set_clock_mode(CM_POSEDGE); ///// get clock mode from master0
    }

    FlowBlockBase::~FlowBlockBase(){
        for (auto basic_node : _basic_nodes){
            delete basic_node;
        }
        for (auto sub_fb : _sub_blocks){
            delete sub_fb;
        }
        for (auto con_fb : _con_blocks){
            delete con_fb;
        }
        for (auto abandon_fb : _abandoned_blocks){
            delete abandon_fb;
        }
        for (auto int_node : _int_nodes){
            delete int_node;
        }
        delete _hold_node;

        delete _force_exit_node;
        delete _flow_sim_engine;
        /////// it is safe to delete nullptr
    }

    FlowBlockBase* FlowBlockBase::gen_implicit_sub_blk(FLOW_BLOCK_TYPE defaultType) {
        /** determine next flow block*/
        FLOW_BLOCK_TYPE next_fb_type = _ctrl->get_top_pattern_flow_block_type();
        if (next_fb_type == DUMMY_BLOCK){
            next_fb_type = defaultType;
        }
        /** create subblock*/
        if ((next_fb_type == PARALLEL_NO_SYN) || (next_fb_type == PARALLEL_AUTO_SYNC)){
            return new FlowBlockParAuto();
        }
        else if (next_fb_type == SEQUENTIAL){
            return new FlowBlockSeq();
        }else{
            assert(false); /** can't determine flow type*/
        }
    }

    void FlowBlockBase::gen_sum_force_exit_node(std::vector<NodeWrap *> &nws) {
        /** check that there is force exit node*/
        for (auto nw : nws){
            _are_there_force_exit |= (nw->get_force_exit_node() != nullptr);
        }

        /** build pseudo node*/
        if (_are_there_force_exit){
            _force_exit_node = new PseudoNode(1, BITWISE_OR);
            for (auto nw : nws){
                if (nw->get_force_exit_node() != nullptr){
                    _force_exit_node->add_depend_node(nw->get_force_exit_node(), nullptr);
                }
            }
            _force_exit_node->assign();
        }
    }

    void FlowBlockBase::fill_intr_rst_signal_to_child(){
        for (auto sub_block_ptr : _sub_blocks){
            assert(sub_block_ptr != nullptr);
            for (auto signal : _int_signals[INT_RESET]){
                sub_block_ptr->add_intr_signal(INT_RESET, signal);
            }
        }

        for (auto con_block_ptr : _con_blocks){
            assert(con_block_ptr != nullptr);
            for (auto signal : _int_signals[INT_RESET]){
                con_block_ptr->add_intr_signal(INT_RESET, signal);
            }
        }
    }

    void FlowBlockBase::gen_intr_node(){
        for (int int_type = 0; int_type < INT_CNT; int_type++){
            if (_int_signals[int_type].empty()){
                continue;
            }
            /// init node
            _int_nodes[int_type] = new OprNode(_int_signals[int_type][0]);

            for (int sig_id = 1; sig_id < _int_signals[int_type].size(); sig_id++){
                _int_nodes[int_type]->add_logic(
                    _int_nodes[int_type]->_value,
                    _int_signals[int_type][sig_id],
                    BITWISE_OR
                );
            }
        }
    }

    Operable* FlowBlockBase::gen_intr_sum_signal(bool is_and_cond, INT_TYPE intr_type){
        std::vector<Operable*>& target_signals = _int_signals[intr_type];

        if (target_signals.empty()){
            return nullptr;
        }
        Operable* result_signal = target_signals[0];
        for (int idx = 1; idx < static_cast<int>(target_signals.size()); idx++){
            mf_assert(target_signals[idx]->getOperableSlice().getSize() == 1,
                      "interrupt signal size cannot more than one");

            result_signal = is_and_cond
                                ? &((*result_signal) & (*target_signals[idx]))
                                : &((*result_signal) | (*target_signals[idx]));
        }
        return result_signal;
    }

    bool FlowBlockBase::is_there_intr_start(){
        return _int_nodes[INT_START] != nullptr;
    }

    bool FlowBlockBase::is_there_intr_rst(){
        return _int_nodes[INT_RESET] != nullptr;
    }

    void FlowBlockBase::fill_hold_signal_to_child(){
        for (auto sub_block_ptr : _sub_blocks){
            assert(sub_block_ptr != nullptr);
            for (auto signal : _hold_signals){
                sub_block_ptr->add_hold_signal(signal);
            }
        }

        for (auto con_block_ptr : _con_blocks){
            assert(con_block_ptr != nullptr);
            for (auto signal : _hold_signals){
                con_block_ptr->add_hold_signal(signal);
            }
        }
    }

    void FlowBlockBase::gen_hold_node(){

        if (_hold_signals.empty()){
            return;
        }
        _hold_node = new OprNode(_hold_signals[0]);

        for (int sig_id = 1; sig_id < static_cast<int>(_hold_signals.size()); sig_id++){
            _hold_node->add_logic(
                _hold_node->_value,
                _hold_signals[sig_id],
                BITWISE_OR
            );
        }

    }

    bool FlowBlockBase::is_there_hold(){
        return _hold_node  != nullptr;
    }

    void FlowBlockBase::set_clock_mode(CLOCK_MODE mode){
        _clk_mode = mode;
        mf_assert((get_clock_mode() == CM_POSEDGE) ||
                      (get_clock_mode() == CM_NEGEDGE)
        , "clock mode in flow must be only CM_POSEDGE or CM_NEGEDGE");
    }

    void FlowBlockBase::build_hw_master(){

        /** override the asmNode     Please remind that it must be done before buildSubHwComponent
         * because sub component such as zif zelse block can put more asmNode(sub block must think by their own)*/

        /** pass the Int reset signal and holding signal to child block*/
        /** dont fill interrupt start signal because this block will start it*/
        fill_intr_rst_signal_to_child();
        fill_hold_signal_to_child();

        /** build the sub block first*/
        build_sub_hw_component();

        /** we so sure now that all sub  Block is ready*/
        /** start build the node for int reset start and hold signal*/
        gen_intr_node();
        gen_hold_node();


        build_hw_component();
    }

    void FlowBlockBase::build_sub_hw_component(){
        for (auto sub_block_ptr : _sub_blocks){
            assert(sub_block_ptr != nullptr);
            sub_block_ptr->build_hw_master();
        }

        for (auto con_block_ptr : _con_blocks){
            assert(con_block_ptr != nullptr);
            con_block_ptr->build_hw_master();
        }

    }

    /**
     * condition such as cif sif celif may receiver condition that has
     * more than 1 in  bitsize, we need to convert it to one in bitsize
     ***/

    Operable* FlowBlockBase::purify_condition(Operable* rawOpr){

            assert(rawOpr != nullptr);
            Slice raw_sl = rawOpr->getOperableSlice();
            assert(raw_sl.getSize() != 0);

            if (raw_sl.getSize() == 1){
                return rawOpr;
            }
            makeVal(autoCmpCondVal, raw_sl.getSize(), 0);
            return &((*rawOpr) > autoCmpCondVal);

    }

    FlowBlockBase* FlowBlockBase::scan_master_join_sub_block(){
        //////// the master join block must have only 1 sub block
        FlowBlockBase* result_fb = nullptr;
        for (FlowBlockBase* sub_fb : _sub_blocks){
            mf_assert(sub_fb != nullptr, "subBlock cannot be nullptr");
            if (sub_fb->is_join_master()){
                mf_assert(result_fb == nullptr, "duplicated master join block");
                result_fb = sub_fb;
            }
        }
        return result_fb;
    }

    std::vector<FlowBlockBase::sortEle> FlowBlockBase::sort_sub_and_con_fb_inorder() {
        std::vector<sortEle> pool_ele;
        /// pool sub block in to the array
        for (int i = 0; i < _sub_blocks.size(); i++){
            pool_ele.push_back({_sub_blocks[i], _sub_blocks_order[i]});
        }
        for (int i = 0; i < _con_blocks.size(); i++){
            pool_ele.push_back({_con_blocks[i], _con_blocks_order[i]});
        }
        /// sort array
        std::sort(pool_ele.begin(), pool_ele.end());

        return pool_ele;
    }

    void FlowBlockBase::override_clock_mode_in_all_asm_nodes(){

        for (Node* node: _basic_nodes){
            assert(node != nullptr);
            assert(node->get_node_type() == ASM_NODE);
            ((AsmNode*)node)->override_clock_mode(get_clock_mode());
        }

    }

}


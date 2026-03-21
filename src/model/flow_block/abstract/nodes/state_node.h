//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_STATENODE_H
#define KATHRYN_STATENODE_H


#include "node.h"
#include "asm_node.h"
#include "logic_node.h"

namespace kathryn{


    /**
     * state that represent status of each register in the circuit
     * */

    struct StateNode : Node{
        StateReg* _state_reg;
        std::vector<AsmNode*> _depend_slave_asm_node; /// the asignment node that depend on this state_node

        explicit StateNode(CLOCK_MODE clk_mode) :
            Node(STATE_NODE),
            _state_reg(new StateReg()){
            add_cycle_related_reg(_state_reg);
            set_clock_mode(clk_mode);
        }

        void make_unset_state_event() override{
            assert(_state_reg != nullptr);
            _state_reg->make_un_set_state_event(get_clock_mode());
        }

        Operable*get_exit_opr_ptr() override{
            assert(_state_reg != nullptr);

            Operable* bined_with_reset_signal =
                bind_with_rst_out_put_if_reset(_state_reg->generate_end_expr());
            ///// hold mean the system is freeze it should not activate the system
            ////////// the systerm should still not unset
            Operable* bined_with_hold_signal =
                bind_with_hold_if_hold(bined_with_reset_signal);

            return bined_with_hold_signal;
        }

        ///// generate the end expression
        Operable*get_state_operating_ptr() override{
            assert(_state_reg != nullptr);
            return _state_reg->generate_end_expr();
        }

        void add_slave_asm_node(AsmNode* asm_node, Operable* cond = nullptr){
            assert(asm_node != nullptr);
            asm_node->add_depend_node(this, cond);
            _depend_slave_asm_node.push_back(asm_node);
        }

        void assign() override{
            _state_reg->set_var_name(ident_name);
            /*** set event*/
            for (auto node_src: node_srcs){
                _state_reg->add_depend_state(node_src.depend_node->get_exit_opr_ptr(), node_src.condition, get_clock_mode());
            }
            if (is_there_hold()){
                _state_reg->add_depend_state(get_state_operating_ptr(), hold_node->get_exit_opr_ptr(), get_clock_mode());
            }

            /** unset event*/
            make_unset_state_event();
            /** slave event*/
            for (AsmNode* asm_node: _depend_slave_asm_node){
                Operable* hold_signal  = is_there_hold()      ? hold_node->get_exit_opr_ptr()
                                                           : nullptr;
                Operable* reset_signal = is_threre_int_reset() ? get_interrupt_reset_ptr()->get_exit_opr_ptr()
                                                           : nullptr;
                asm_node->assign_from_state_node(hold_signal, reset_signal);
            }
        }

        int get_cycle_used() override{
            ////// incase holding signal the system cannot know when it is finish
            return is_there_hold() ? NODE_CYCLE_USED_UNKNOWN : 1;
        }
    };

    /**
         * node that represent syn status of each register in the circuit
         * usually used in parallel block with unknown exact cycle
         * */

    struct SynNode : Node{
        SyncReg* _syn_reg;
        PseudoNode* _force_exit_node = nullptr;
        ////// sync node not require the holdsignal
        ////// we realize that if the state is not activate SynNode the syn_node will remain the same state

        /**in SynNode condition and depend_state is disengage*/
        explicit SynNode(int syn_size, CLOCK_MODE clk_mode) :
            Node(SYN_NODE),
            _syn_reg(new SyncReg(syn_size)){
            add_cycle_related_reg(_syn_reg);
            set_clock_mode(clk_mode);
        }

        void make_unset_state_event() override{
            assert(_syn_reg != nullptr);
            _syn_reg->make_un_set_state_event(get_clock_mode());
        }

        void make_user_reset_event() override{
            if (is_threre_int_reset()) {
                _syn_reg->make_user_rst_event(int_reset->get_exit_opr_ptr(), get_clock_mode());
            }
            if (_force_exit_node != nullptr){
                _syn_reg->make_user_rst_event(_force_exit_node->get_exit_opr_ptr(), get_clock_mode());
            }
        }

        Operable*get_exit_opr_ptr() override{return bind_with_rst_out_put_if_reset(_syn_reg->generate_end_expr());}

        void set_force_exit_event(PseudoNode* nd){
            assert(nd != nullptr);
            _force_exit_node = nd;
        }

        void assign() override{
            _syn_reg->set_var_name(ident_name);
            /** make start event*/
            Operable* not_force_exit  = nullptr;
            if (_force_exit_node){
                not_force_exit = &(~(*_force_exit_node->get_exit_opr_ptr()));
            }
            for (auto depend_node : node_srcs){
                assert(depend_node.condition == nullptr);
                _syn_reg->add_depend_state(depend_node.depend_node->get_exit_opr_ptr(), not_force_exit, get_clock_mode());
            }
            /** make unset event*/
            make_unset_state_event();
            make_user_reset_event();
        }

        int get_cycle_used() override{ return 1; }

        bool is_state_full_node() override { return false;}

    };



}


#endif //KATHRYN_STATENODE_H

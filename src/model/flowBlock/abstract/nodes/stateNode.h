//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_STATENODE_H
#define KATHRYN_STATENODE_H


#include "node.h"
#include "asmNode.h"
#include "logicNode.h"

namespace kathryn{


    /**
     * state that represent status of each register in the circuit
     * */

    struct StateNode : Node{
        StateReg*             _state_reg;
        std::vector<AsmNode*> _depend_slave_AsmNode; /// the asignment node that depend on this stateNode

        explicit StateNode(CLOCK_MODE clkMode) :
            Node     (STATE_NODE),
            _state_reg(new StateReg()){
            add_cycle_related_reg(_state_reg);
            set_clock_mode(clkMode);
        }

        void make_unset_state_event() override{
            assert(_state_reg != nullptr);
            _state_reg->makeUnSetStateEvent(get_clock_mode());
        }

        Operable* get_exit_opr_ptr() override{
            assert(_state_reg != nullptr);

            Operable* bined_with_reset_signal =
                bind_with_rst_output_if_reset(_state_reg->generateEndExpr());
            ///// hold mean the system is freeze it should not activate the system
            ////////// the systerm should still not unset
            Operable* bined_with_hold_signal =
                bind_with_hold_if_hold(bined_with_reset_signal);

            return bined_with_hold_signal;
        }

        ///// generate the end expression
        Operable* get_operating_state_ptr() override{
            assert(_state_reg != nullptr);
            return _state_reg->generateEndExpr();
        }

        void add_slave_asm_node(AsmNode* asmNode, Operable* cond = nullptr){
            assert(asmNode != nullptr);
            asmNode->add_depend_node(this, cond);
            _depend_slave_AsmNode.push_back(asmNode);
        }

        void assign() override{
            _state_reg->setVarName(_ident_name);
            /*** set event*/
            for (auto nodeSrc: _node_srcs){
                _state_reg->addDependState(nodeSrc.depend_node->get_exit_opr_ptr(), nodeSrc.condition, get_clock_mode());
            }
            if (is_there_hold()){
                _state_reg->addDependState(get_operating_state_ptr(), _hold_node->get_exit_opr_ptr(), get_clock_mode());
            }

            /** unset event*/
            make_unset_state_event();
            /** slave event*/
            for (AsmNode* asmNode: _depend_slave_AsmNode){
                Operable* holdSignal  = is_there_hold()      ? _hold_node->get_exit_opr_ptr()
                                                           : nullptr;
                Operable* resetSignal = is_threre_int_reset() ? get_interrupt_reset_ptr()->get_exit_opr_ptr()
                                                           : nullptr;
                asmNode->assign_from_state_node(holdSignal, resetSignal);
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
        SyncReg* _synReg;
        PseudoNode* _force_exit_node = nullptr;
        ////// sync node not require the holdsignal
        ////// we realize that if the state is not activate SynNode the synNode will remain the same state

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int syn_size, CLOCK_MODE clk_mode) :
            Node(SYN_NODE),
            _synReg(new SyncReg(syn_size)){
            add_cycle_related_reg(_synReg);
            set_clock_mode(clk_mode);
        }

        void make_unset_state_event() override{
            assert(_synReg != nullptr);
            _synReg->makeUnSetStateEvent(get_clock_mode());
        }

        void make_user_reset_event() override{
            if (is_threre_int_reset()) {
                _synReg->makeUserRstEvent(_int_reset->get_exit_opr_ptr(), get_clock_mode());
            }
            if (_force_exit_node != nullptr){
                _synReg->makeUserRstEvent(_force_exit_node->get_exit_opr_ptr(), get_clock_mode());
            }
        }

        Operable* get_exit_opr_ptr() override{return bind_with_rst_output_if_reset(_synReg->generateEndExpr());}

        void set_force_exit_event(PseudoNode* nd){
            assert(nd != nullptr);
            _force_exit_node = nd;
        }

        void assign() override{
            _synReg->setVarName(_ident_name);
            /** make start event*/
            Operable* not_force_exit  = nullptr;
            if (_force_exit_node){
                not_force_exit = &(~(*_force_exit_node->get_exit_opr_ptr()));
            }
            for (auto dependNode : _node_srcs){
                assert(dependNode.condition == nullptr);
                _synReg->addDependState(dependNode.depend_node->get_exit_opr_ptr(), not_force_exit, get_clock_mode());
            }
            /** make unset event*/
            make_unset_state_event();
            make_user_reset_event();
        }

        int get_cycle_used() override{ return 1; }

        bool is_stateful_node() override { return false;}

    };



}


#endif //KATHRYN_STATENODE_H

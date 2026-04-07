//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_WAITNODE_H
#define KATHRYN_WAITNODE_H

#include "node.h"
#include "model/flowBlock/abstract/spReg/waitReg.h"

namespace kathryn{

    struct WaitCondNode : Node{

        CondWaitStateReg* _cond_wait_state_reg = nullptr;

        explicit WaitCondNode(Operable* wait_cond, CLOCK_MODE clock_mode):
                Node(WAITCOND_NODE){
            assert(wait_cond != nullptr);
            _cond_wait_state_reg = new CondWaitStateReg(wait_cond);
            add_cycle_related_reg(_cond_wait_state_reg);
            set_clock_mode(clock_mode);
        }

        void make_unset_state_event() override{
            assert(_cond_wait_state_reg != nullptr);
            _cond_wait_state_reg->makeUnSetStateEvent(get_clock_mode());
        }

        void make_user_reset_event() override{
            if (is_threre_int_reset()){
                _cond_wait_state_reg->makeUserRstEvent(_int_reset->get_exit_opr_ptr(), get_clock_mode());
            }

        }

        Operable* get_operating_state_ptr() override{
            assert(_cond_wait_state_reg != nullptr);
            return _cond_wait_state_reg->generateEndExpr();
        }

        Operable* get_exit_opr_ptr() override{
            assert(_cond_wait_state_reg != nullptr);
            Operable* bined_with_reset_sig =
                bind_with_rst_output_if_reset(_cond_wait_state_reg->generateEndExpr());
            Operable* bined_with_hold_sig =
                bind_with_hold_if_hold(bined_with_reset_sig);
            return bined_with_hold_sig;
        }

        void assign() override{
            assert(!_node_srcs.empty());
            for(auto node_src: _node_srcs){
                _cond_wait_state_reg->addDependState(node_src.depend_node->get_exit_opr_ptr(), node_src.condition, get_clock_mode());
            }
            if (is_there_hold()){
                _cond_wait_state_reg->addDependState(get_operating_state_ptr(), _hold_node->get_exit_opr_ptr(), get_clock_mode());
            }

            make_unset_state_event();
            make_user_reset_event();
            _cond_wait_state_reg->setVarName(_ident_name);
        }

        int get_cycle_used() override {return NODE_CYCLE_USED_UNKNOWN;}

    };

    struct WaitCycleNode : Node{
        int _cycle = -1;
        CycleWaitStateReg* _cycle_wait_state_reg = nullptr;

        explicit WaitCycleNode(int cycle, CLOCK_MODE clock_mode):
                Node(WAITCYCLE_NODE),
                _cycle(cycle){

            _cycle_wait_state_reg = new CycleWaitStateReg(cycle);
            add_cycle_related_reg(_cycle_wait_state_reg);
            set_clock_mode(clock_mode);
        }

        explicit WaitCycleNode(Operable* opr1, CLOCK_MODE clock_mode):
                Node(WAITCYCLE_NODE)
        {
            _cycle_wait_state_reg = new CycleWaitStateReg(opr1);
            add_cycle_related_reg(_cycle_wait_state_reg);
            set_clock_mode(clock_mode);
        }

        void make_unset_state_event() override{
            assert(_cycle_wait_state_reg != nullptr);
            _cycle_wait_state_reg->makeUnSetStateEvent(get_clock_mode());
        }

        void make_user_reset_event() override{
            if(is_threre_int_reset()){
                _cycle_wait_state_reg->makeUserRstEvent(_int_reset->get_exit_opr_ptr(), get_clock_mode());
            }

        }

        Operable* get_exit_opr_ptr() override{
            assert(_cycle_wait_state_reg != nullptr);
            Operable* bined_with_reset_signal =
                bind_with_rst_output_if_reset(_cycle_wait_state_reg->generateEndExpr());
            Operable* bined_with_hold_signal =
                bind_with_hold_if_hold(bined_with_reset_signal);
            return bined_with_hold_signal;
        }

        void assign() override{

            /**normal start event*/
            for(auto node_src: _node_srcs){
                _cycle_wait_state_reg->addDependState(node_src.depend_node->get_exit_opr_ptr(), node_src.condition, get_clock_mode());
            }
            /** inc event*/
            if (is_there_hold()){
                _cycle_wait_state_reg->makeIncStateEvent(_hold_node->get_exit_opr_ptr(), get_clock_mode());
            }else{
                _cycle_wait_state_reg->makeIncStateEvent(nullptr, get_clock_mode());
            }
            /** unset event*/
            make_unset_state_event();
            make_user_reset_event();
            _cycle_wait_state_reg->setVarName(_ident_name);
        }

        int get_cycle_used() override{
            if (is_there_hold()){
                return IN_CONSIST_CYCLE_USED;
            }
            return _cycle;
        }

    };

}

#endif //KATHRYN_WAITNODE_H

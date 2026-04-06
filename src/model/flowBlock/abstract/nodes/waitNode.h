//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_WAITNODE_H
#define KATHRYN_WAITNODE_H

#include "node.h"
#include "model/flowBlock/abstract/spReg/waitReg.h"

namespace kathryn{

    struct WaitCondNode : Node{

        CondWaitStateReg* _condWaitStateReg = nullptr;

        explicit WaitCondNode(Operable* waitCond, CLOCK_MODE clockMode):
                Node(WAITCOND_NODE){
            assert(waitCond != nullptr);
            _condWaitStateReg = new CondWaitStateReg(waitCond);
            add_cycle_related_reg(_condWaitStateReg);
            set_clock_mode(clockMode);
        }

        void make_unset_state_event() override{
            assert(_condWaitStateReg != nullptr);
            _condWaitStateReg->makeUnSetStateEvent(get_clock_mode());
        }

        void make_user_reset_event() override{
            if (is_threre_int_reset()){
                _condWaitStateReg->makeUserRstEvent(_int_reset->get_exit_opr_ptr(), get_clock_mode());
            }

        }

        Operable* get_operating_state_ptr() override{
            assert(_condWaitStateReg != nullptr);
            return _condWaitStateReg->generateEndExpr();
        }

        Operable* get_exit_opr_ptr() override{
            assert(_condWaitStateReg != nullptr);
            Operable* binedWithResetSig =
                bind_with_rst_out_put_if_reset(_condWaitStateReg->generateEndExpr());
            Operable* binedWithHoldSig =
                bind_with_hold_if_hold(binedWithResetSig);
            return binedWithHoldSig;
        }

        void assign() override{
            assert(!_node_srcs.empty());
            for(auto nodeSrc: _node_srcs){
                _condWaitStateReg->addDependState(nodeSrc.dependNode->get_exit_opr_ptr(), nodeSrc.condition, get_clock_mode());
            }
            if (is_there_hold()){
                _condWaitStateReg->addDependState(get_operating_state_ptr(), _hold_node->get_exit_opr_ptr(), get_clock_mode());
            }

            make_unset_state_event();
            make_user_reset_event();
            _condWaitStateReg->setVarName(_ident_name);
        }

        int get_cycle_used() override {return NODE_CYCLE_USED_UNKNOWN;}

    };

    struct WaitCycleNode : Node{
        int _cycle = -1;
        CycleWaitStateReg* _cycleWaitStateReg = nullptr;

        explicit WaitCycleNode(int cycle, CLOCK_MODE clockMode):
                Node(WAITCYCLE_NODE),
                _cycle(cycle){

            _cycleWaitStateReg = new CycleWaitStateReg(cycle);
            add_cycle_related_reg(_cycleWaitStateReg);
            set_clock_mode(clockMode);
        }

        explicit WaitCycleNode(Operable* opr1, CLOCK_MODE clockMode):
                Node(WAITCYCLE_NODE)
        {
            _cycleWaitStateReg = new CycleWaitStateReg(opr1);
            add_cycle_related_reg(_cycleWaitStateReg);
            set_clock_mode(clockMode);
        }

        void make_unset_state_event() override{
            assert(_cycleWaitStateReg != nullptr);
            _cycleWaitStateReg->makeUnSetStateEvent(get_clock_mode());
        }

        void make_user_reset_event() override{
            if(is_threre_int_reset()){
                _cycleWaitStateReg->makeUserRstEvent(_int_reset->get_exit_opr_ptr(), get_clock_mode());
            }

        }

        Operable* get_exit_opr_ptr() override{
            assert(_cycleWaitStateReg != nullptr);
            Operable* binedWithResetSignal =
                bind_with_rst_out_put_if_reset(_cycleWaitStateReg->generateEndExpr());
            Operable* binedWithHoldSignal =
                bind_with_hold_if_hold(binedWithResetSignal);
            return binedWithHoldSignal;
        }

        void assign() override{

            /**normal start event*/
            for(auto nodeSrc: _node_srcs){
                _cycleWaitStateReg->addDependState(nodeSrc.dependNode->get_exit_opr_ptr(), nodeSrc.condition, get_clock_mode());
            }
            /** inc event*/
            if (is_there_hold()){
                _cycleWaitStateReg->makeIncStateEvent(_hold_node->get_exit_opr_ptr(), get_clock_mode());
            }else{
                _cycleWaitStateReg->makeIncStateEvent(nullptr, get_clock_mode());
            }
            /** unset event*/
            make_unset_state_event();
            make_user_reset_event();
            _cycleWaitStateReg->setVarName(_ident_name);
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

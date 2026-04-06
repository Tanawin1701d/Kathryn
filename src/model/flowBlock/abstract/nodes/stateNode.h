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
        StateReg* _stateReg;
        std::vector<AsmNode*> _dependSlaveAsmNode; /// the asignment node that depend on this stateNode

        explicit StateNode(CLOCK_MODE clkMode) :
            Node(STATE_NODE),
            _stateReg(new StateReg()){
            add_cycle_related_reg(_stateReg);
            set_clock_mode(clkMode);
        }

        void make_unset_state_event() override{
            assert(_stateReg != nullptr);
            _stateReg->makeUnSetStateEvent(get_clock_mode());
        }

        Operable* get_exit_opr_ptr() override{
            assert(_stateReg != nullptr);

            Operable* binedWithResetSignal =
                bind_with_rst_out_put_if_reset(_stateReg->generateEndExpr());
            ///// hold mean the system is freeze it should not activate the system
            ////////// the systerm should still not unset
            Operable* binedWithHoldSignal =
                bind_with_hold_if_hold(binedWithResetSignal);

            return binedWithHoldSignal;
        }

        ///// generate the end expression
        Operable* get_operating_state_ptr() override{
            assert(_stateReg != nullptr);
            return _stateReg->generateEndExpr();
        }

        void addSlaveAsmNode(AsmNode* asmNode, Operable* cond = nullptr){
            assert(asmNode != nullptr);
            asmNode->add_depend_node(this, cond);
            _dependSlaveAsmNode.push_back(asmNode);
        }

        void assign() override{
            _stateReg->setVarName(_ident_name);
            /*** set event*/
            for (auto nodeSrc: _node_srcs){
                _stateReg->addDependState(nodeSrc.dependNode->get_exit_opr_ptr(), nodeSrc.condition, get_clock_mode());
            }
            if (is_there_hold()){
                _stateReg->addDependState(get_operating_state_ptr(), _hold_node->get_exit_opr_ptr(), get_clock_mode());
            }

            /** unset event*/
            make_unset_state_event();
            /** slave event*/
            for (AsmNode* asmNode: _dependSlaveAsmNode){
                Operable* holdSignal  = is_there_hold()      ? _hold_node->get_exit_opr_ptr()
                                                           : nullptr;
                Operable* resetSignal = is_threre_int_reset() ? get_interrupt_reset_ptr()->get_exit_opr_ptr()
                                                           : nullptr;
                asmNode->assignFromStateNode(holdSignal, resetSignal);
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
        PseudoNode* _forceExitNode = nullptr;
        ////// sync node not require the holdsignal
        ////// we realize that if the state is not activate SynNode the synNode will remain the same state

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize, CLOCK_MODE clkMode) :
            Node(SYN_NODE),
            _synReg(new SyncReg(synSize)){
            add_cycle_related_reg(_synReg);
            set_clock_mode(clkMode);
        }

        void make_unset_state_event() override{
            assert(_synReg != nullptr);
            _synReg->makeUnSetStateEvent(get_clock_mode());
        }

        void make_user_reset_event() override{
            if (is_threre_int_reset()) {
                _synReg->makeUserRstEvent(_int_reset->get_exit_opr_ptr(), get_clock_mode());
            }
            if (_forceExitNode != nullptr){
                _synReg->makeUserRstEvent(_forceExitNode->get_exit_opr_ptr(), get_clock_mode());
            }
        }

        Operable* get_exit_opr_ptr() override{return bind_with_rst_out_put_if_reset(_synReg->generateEndExpr());}

        void setForceExitEvent(PseudoNode* nd){
            assert(nd != nullptr);
            _forceExitNode = nd;
        }

        void assign() override{
            _synReg->setVarName(_ident_name);
            /** make start event*/
            Operable* notForceExit  = nullptr;
            if (_forceExitNode){
                notForceExit = &(~(*_forceExitNode->get_exit_opr_ptr()));
            }
            for (auto dependNode : _node_srcs){
                assert(dependNode.condition == nullptr);
                _synReg->addDependState(dependNode.dependNode->get_exit_opr_ptr(), notForceExit, get_clock_mode());
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

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
        StateReg* _state_reg;
        std::vector<AsmNode*> _depend_slave_asm_node; /// the asignment node that depend on this stateNode

        explicit StateNode(CLOCK_MODE clkMode) :
            Node(STATE_NODE),
            _state_reg(new StateReg()){
            addCycleRelatedReg(_state_reg);
            setClockMode(clkMode);
        }

        void makeUnsetStateEvent() override{
            assert(_state_reg != nullptr);
            _state_reg->makeUnSetStateEvent(getClockMode());
        }

        Operable*getExitOprPtr() override{
            assert(_state_reg != nullptr);

            Operable* binedWithResetSignal =
                bindWithRstOutPutIfReset(_state_reg->generateEndExpr());
            ///// hold mean the system is freeze it should not activate the system
            ////////// the systerm should still not unset
            Operable* binedWithHoldSignal =
                bindWithHoldIfHold(binedWithResetSignal);

            return binedWithHoldSignal;
        }

        ///// generate the end expression
        Operable*getStateOperatingPtr() override{
            assert(_state_reg != nullptr);
            return _state_reg->generateEndExpr();
        }

        void addSlaveAsmNode(AsmNode* asmNode, Operable* cond = nullptr){
            assert(asmNode != nullptr);
            asmNode->addDependNode(this, cond);
            _depend_slave_asm_node.pushBack(asmNode);
        }

        void assign() override{
            _state_reg->setVarName(identName);
            /*** set event*/
            for (auto nodeSrc: nodeSrcs){
                _state_reg->addDependState(nodeSrc.dependNode->getExitOprPtr(), nodeSrc.condition, getClockMode());
            }
            if (isThereHold()){
                _state_reg->addDependState(getStateOperatingPtr(), holdNode->getExitOprPtr(), getClockMode());
            }

            /** unset event*/
            makeUnsetStateEvent();
            /** slave event*/
            for (AsmNode* asmNode: _depend_slave_asm_node){
                Operable* holdSignal  = isThereHold()      ? holdNode->getExitOprPtr()
                                                           : nullptr;
                Operable* resetSignal = isThrereIntReset() ? getInterruptResetPtr()->getExitOprPtr()
                                                           : nullptr;
                asmNode->assignFromStateNode(holdSignal, resetSignal);
            }
        }

        int getCycleUsed() override{
            ////// incase holding signal the system cannot know when it is finish
            return isThereHold() ? NODE_CYCLE_USED_UNKNOWN : 1;
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
        ////// we realize that if the state is not activate SynNode the synNode will remain the same state

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize, CLOCK_MODE clkMode) :
            Node(SYN_NODE),
            _syn_reg(new SyncReg(synSize)){
            addCycleRelatedReg(_syn_reg);
            setClockMode(clkMode);
        }

        void makeUnsetStateEvent() override{
            assert(_syn_reg != nullptr);
            _syn_reg->makeUnSetStateEvent(getClockMode());
        }

        void makeUserResetEvent() override{
            if (isThrereIntReset()) {
                _syn_reg->makeUserRstEvent(intReset->getExitOprPtr(), getClockMode());
            }
            if (_force_exit_node != nullptr){
                _syn_reg->makeUserRstEvent(_force_exit_node->getExitOprPtr(), getClockMode());
            }
        }

        Operable*getExitOprPtr() override{return bindWithRstOutPutIfReset(_syn_reg->generateEndExpr());}

        void setForceExitEvent(PseudoNode* nd){
            assert(nd != nullptr);
            _force_exit_node = nd;
        }

        void assign() override{
            _syn_reg->setVarName(identName);
            /** make start event*/
            Operable* notForceExit  = nullptr;
            if (_force_exit_node){
                notForceExit = &(~(*_force_exit_node->getExitOprPtr()));
            }
            for (auto dependNode : nodeSrcs){
                assert(dependNode.condition == nullptr);
                _syn_reg->addDependState(dependNode.dependNode->getExitOprPtr(), notForceExit, getClockMode());
            }
            /** make unset event*/
            makeUnsetStateEvent();
            makeUserResetEvent();
        }

        int getCycleUsed() override{ return 1; }

        bool isStateFullNode() override { return false;}

    };



}


#endif //KATHRYN_STATENODE_H

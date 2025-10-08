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
            addCycleRelatedReg(_stateReg);
            setClockMode(clkMode);
        }

        void makeUnsetStateEvent() override{
            assert(_stateReg != nullptr);
            _stateReg->makeUnSetStateEvent(getClockMode());
        }

        Operable* getExitOpr() override{
            assert(_stateReg != nullptr);

            Operable* binedWithResetSignal =
                bindWithRstOutPutIfReset(_stateReg->generateEndExpr());
            ///// hold mean the system is freeze it should not activate the system
            ////////// the systerm should still not unset
            Operable* binedWithHoldSignal =
                bindWithHoldIfHold(binedWithResetSignal);

            return binedWithHoldSignal;
        }

        ///// generate the end expression
        Operable* getStateOperating() override{
            assert(_stateReg != nullptr);
            return _stateReg->generateEndExpr();
        }

        void addSlaveAsmNode(AsmNode* asmNode, Operable* cond = nullptr){
            assert(asmNode != nullptr);
            asmNode->addDependNode(this, cond);
            _dependSlaveAsmNode.push_back(asmNode);
        }

        void assign() override{
            _stateReg->setVarName(identName);
            /*** set event*/
            for (auto nodeSrc: nodeSrcs){
                _stateReg->addDependState(nodeSrc.dependNode->getExitOpr(), nodeSrc.condition, getClockMode());
            }
            if (isThereHold()){
                _stateReg->addDependState(getStateOperating(), holdNode->getExitOpr(), getClockMode());
            }

            /** unset event*/
            makeUnsetStateEvent();
            /** slave event*/
            for (AsmNode* asmNode: _dependSlaveAsmNode){
                Operable* holdSignal = isThereHold() ? holdNode->getExitOpr(): nullptr;
                asmNode->assignFromStateNode(holdSignal);
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
        SyncReg* _synReg;
        PseudoNode* _forceExitNode = nullptr;
        ////// sync node not require the holdsignal
        ////// we realize that if the state is not activate SynNode the synNode will remain the same state

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize, CLOCK_MODE clkMode) :
            Node(SYN_NODE),
            _synReg(new SyncReg(synSize)){
            addCycleRelatedReg(_synReg);
            setClockMode(clkMode);
        }

        void makeUnsetStateEvent() override{
            assert(_synReg != nullptr);
            _synReg->makeUnSetStateEvent(getClockMode());
        }

        void makeUserResetEvent() override{
            if (isThrereIntReset()) {
                _synReg->makeUserRstEvent(intReset->getExitOpr(), getClockMode());
            }
            if (_forceExitNode != nullptr){
                _synReg->makeUserRstEvent(_forceExitNode->getExitOpr(), getClockMode());
            }
        }

        Operable* getExitOpr() override{return bindWithRstOutPutIfReset(_synReg->generateEndExpr());}

        void setForceExitEvent(PseudoNode* nd){
            assert(nd != nullptr);
            _forceExitNode = nd;
        }

        void assign() override{
            _synReg->setVarName(identName);
            /** make start event*/
            Operable* notForceExit  = nullptr;
            if (_forceExitNode){
                notForceExit = &(~(*_forceExitNode->getExitOpr()));
            }
            for (auto dependNode : nodeSrcs){
                assert(dependNode.condition == nullptr);
                _synReg->addDependState(dependNode.dependNode->getExitOpr(), notForceExit, getClockMode());
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

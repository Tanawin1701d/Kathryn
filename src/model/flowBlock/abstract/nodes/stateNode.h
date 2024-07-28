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

        explicit StateNode() :
            Node(STATE_NODE),
            _stateReg(new StateReg()){
            addCycleRelatedReg(_stateReg);
        }

        void makeUnsetStateEvent() override{
            assert(_stateReg != nullptr);
            _stateReg->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{
            assert(_stateReg != nullptr);
            return bindWithRstOutPutIfReset(_stateReg->generateEndExpr());
        }

        Operable* getStateOperating() override{
            assert(_stateReg != nullptr);
            return _stateReg->generateEndExpr();
        }

        void addSlaveAsmNode(AsmNode* asmNode){
            assert(asmNode != nullptr);
            asmNode->addDependNode(this, nullptr);
            _dependSlaveAsmNode.push_back(asmNode);
        }

        void assign() override{
            _stateReg->setVarName(identName);
            /*** set event*/
            for (auto nodeSrc: nodeSrcs){
                _stateReg->addDependState(nodeSrc.dependNode->getExitOpr(), nodeSrc.condition);
            }
            /** unset event*/
            makeUnsetStateEvent();
            /** slave event*/
            for (AsmNode* asmNode: _dependSlaveAsmNode){
                asmNode->assignFromStateNode();
            }
        }

        int getCycleUsed() override {return 1;}
    };

    /**
         * node that represent syn status of each register in the circuit
         * usually used in parallel block with unknown exact cycle
         * */

    struct SynNode : Node{
        SyncReg* _synReg;
        PseudoNode* _forceExitNode = nullptr;

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize) :
            Node(SYN_NODE),
            _synReg(new SyncReg(synSize)){

            addCycleRelatedReg(_synReg);
        }

        void makeUnsetStateEvent() override{
            assert(_synReg != nullptr);
            _synReg->makeUnSetStateEvent();
        }

        void makeUserResetEvent() override{
            if (isThrereIntReset()) {
                _synReg->makeUserRstEvent(intReset->getExitOpr());
            }
            if (_forceExitNode != nullptr){
                _synReg->makeUserRstEvent(_forceExitNode->getExitOpr());
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
                _synReg->addDependState(dependNode.dependNode->getExitOpr(), notForceExit);
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

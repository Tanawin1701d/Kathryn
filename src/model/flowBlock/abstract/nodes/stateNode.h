//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_STATENODE_H
#define KATHRYN_STATENODE_H


#include "node.h"
#include "asmNode.h"

namespace kathryn{


    /**
     * state that represent status of each register in the circuit
     * */

    struct StateNode : Node{
        StateReg* _stateReg;
        std::vector<AsmNode*> _dependSlaveAsmNode; /// the asignment node that depend on this stateNode

        explicit StateNode() :
            Node(STATE_NODE),
            _stateReg(new StateReg()){}

        void makeUnsetStateEvent() override{
            assert(_stateReg != nullptr);
            _stateReg->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{
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

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_stateReg != nullptr);
            bool isStateSet = _stateReg->getRtlValItf()->getCurVal().getLogicalValue();
            if (isStateSet){
                setBlockOrNodeRunning();
                incEngine();
            }

        }
    };

    /**
         * node that represent syn status of each register in the circuit
         * usually used in parallel block with unknown exact cycle
         * */

    struct SynNode : Node{
        SyncReg* _synReg;

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize) :
            Node(SYN_NODE),
            _synReg(new SyncReg(synSize)){}

        void makeUnsetStateEvent() override{
            assert(_synReg != nullptr);
            _synReg->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{return _synReg->generateEndExpr();}

        void assign() override{
            _synReg->setVarName(identName);
            /** make start event*/
            for (auto dependNode : nodeSrcs){
                assert(dependNode.condition == nullptr);
                _synReg->addDependState(dependNode.dependNode->getExitOpr(), nullptr);
            }
            /** make unset event*/
            makeUnsetStateEvent();
        }

        int getCycleUsed() override{ return 1; }

        bool isStateFullNode() override { return false;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_synReg != nullptr);
            bool isFullSyncCycle = _synReg->isSimAtFullSyn();
            /** inc engine*/
            if (isFullSyncCycle){
                setBlockOrNodeRunning();
                incEngine();
            }

        }

    };



}


#endif //KATHRYN_STATENODE_H

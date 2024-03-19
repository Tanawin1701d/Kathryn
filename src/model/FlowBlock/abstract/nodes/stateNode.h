//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_STATENODE_H
#define KATHRYN_STATENODE_H


#include "node.h"

namespace kathryn{


    /**
     * state that represent status of each register in the circuit
     * */

    struct StateNode : Node{
        StateReg* _stateReg;

        explicit StateNode() :
            Node(STATE_NODE),
            _stateReg(new StateReg()){}

        Node* clone() override{
            auto clNode = new StateNode(*this);
            clNode->setCpyPtr(this);
            return clNode;
        }

        void makeUnsetStateEvent() override{
            assert(_stateReg != nullptr);
            _stateReg->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{
            assert(_stateReg != nullptr);
            return _stateReg->generateEndExpr();
        }

        void assign() override{
            Operable* dependNodeOpr = transformAllDepNodeToOpr();
            assert(dependNodeOpr != nullptr);
            _stateReg->addDependState(dependNodeOpr, condition);
            makeUnsetStateEvent();
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

        Node* clone() override{
            /** syn node is not supposed to be copied*/
            assert(false);
        }

        void addCondtion(Operable* opr, LOGIC_OP op) override{ assert(false);}

        void makeUnsetStateEvent() override{
            assert(_synReg != nullptr);
            _synReg->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{return _synReg->generateEndExpr();}

        void assign() override{
            for (auto dependNode : dependNodes){
                _synReg->addDependState(dependNode->getExitOpr(), condition);
            }
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

    struct PseudoNode : Node{
        expression* _pseudoAssignMeta = nullptr;

        explicit PseudoNode(int expr_size) :
            Node(PSEUDO_NODE),
            _pseudoAssignMeta(new expression(expr_size)){}

        Node* clone() override{
            auto clNode = new PseudoNode(*this);
            clNode->setCpyPtr(this);
            return clNode;
        }

        void assign() override{
            if (condition == nullptr)
                *_pseudoAssignMeta = *transformAllDepNodeToOpr();
            else
                *_pseudoAssignMeta = (*transformAllDepNodeToOpr()) & (*condition);
            assert(_pseudoAssignMeta != nullptr);
        }
        int getCycleUsed() override { return 0; }

        Operable* getExitOpr() override{return _pseudoAssignMeta;}

        bool isStateFullNode() override{ return false; }

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_pseudoAssignMeta != nullptr);
            /** inc engine do not increase engine*/

        }



    };

    struct DummyNode : Node{
        Val* _value = nullptr;

        explicit DummyNode(Val* value) :
            Node(DUMMY_NODE),
            _value(value){
            assert(_value != nullptr);
        }

        Node* clone() override{
            auto clNode = new DummyNode(*this);
            clNode->setCpyPtr(this);
            return clNode;
        }

        void assign() override{
            /** we don't support assign from condition or depend state*/
            assert(dependNodes.empty());
            assert(condition == nullptr);
        }

        int getCycleUsed() override{ return 0; }

        Operable* getExitOpr() override{return _value;}

        bool isStateFullNode() override{return false;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_value != nullptr);
            /** inc engine do not increase engine*/
        }

    };

}


#endif //KATHRYN_STATENODE_H

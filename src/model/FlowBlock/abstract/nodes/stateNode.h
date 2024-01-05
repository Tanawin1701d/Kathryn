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
            Node(),
            _stateReg(new StateReg()){}

        Node* clone() override{
            auto clNode = new StateNode(*this);
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
            auto dependNodeOpr = getAllDependNodeOpr();
            assert(dependNodeOpr != nullptr);
            _stateReg->addDependState(dependNodeOpr, condition);
            makeUnsetStateEvent();
        }

        int getCycleUsed() override {return 1;}
    };

    /**
         * node that represent syn status of each register in the circuit
         * usually used in parallel block with unknown exact cycle
         * */

    struct SynNode : Node{
        StateReg* _synReg;

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize) : _synReg(new StateReg(synSize)){}

        Node* clone() override{
            /** syn node is not supposed to be copy*/
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

    };

    struct PseudoNode : Node{
        expression* _pseudoAssignMeta = nullptr;

        explicit PseudoNode() :
            Node(),
            _pseudoAssignMeta(new expression()){}

        Node* clone() override{
            auto clNode = new PseudoNode(*this);
            return clNode;
        }

        void assign() override{
            _pseudoAssignMeta = &(*getAllDependNodeOpr() & *condition);
        }
        int getCycleUsed() override { return 0; };

        bool isStateFullNode() override{ return false; }

    };
}


#endif //KATHRYN_STATENODE_H

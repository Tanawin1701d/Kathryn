//
// Created by tanawin on 6/1/2567.
//

#ifndef KATHRYN_STARTNODE_H
#define KATHRYN_STARTNODE_H

#include "node.h"
#include "model/hwComponent/abstract/makeComponent.h"

namespace kathryn{

    struct StartNode : Node{
        makeVal(upState  , 1, 1);
        makeVal(downState, 1, 0);

        StateReg* _startState = nullptr;
        Operable* _rstSig     = nullptr;

        explicit StartNode(Operable* rstSig):
                Node(START_NODE),
                _startState(new StateReg(false)),
                _rstSig(rstSig){
                assert(_rstSig != nullptr);
        }

        Node* clone() override{
            auto clNode = new StartNode(*this);
            clNode->setCpyPtr(this);
            return clNode;
        }

        void makeUnsetStateEvent() override{
            assert(_startState != nullptr);
            _startState->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{
            assert(_rstSig != nullptr);
            return &(*_startState == upState);
        }

        void assign() override{
            _startState->addDependState(_rstSig, nullptr);
            makeUnsetStateEvent();
            /**no need to reset due to it used*/
        }

        int getCycleUsed() override{
            return 1;
        }

        void simStartCurCycle() override{
            if (isCurCycleSimulated()){
                return;
            }
        }

    };

}

#endif //KATHRYN_STARTNODE_H

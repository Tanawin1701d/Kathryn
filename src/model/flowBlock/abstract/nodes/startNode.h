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
            _startState->setVarName("startNode");
            /**no need to reset due to it used*/
        }

        int getCycleUsed() override{
            return 1;
        }

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
        }

    };

}

#endif //KATHRYN_STARTNODE_H

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

        void finalize() override{

            assert(checkAllDepEmpty());
            _startState->setVarName("startNode");
            _startState->addDependState(_rstSig, nullptr);
            _startState->makeUnsetEvent();
        }

        Operable* getExitOpr() override{
            assert(_rstSig != nullptr);
            return &(*_startState == upState);
        }

        int getCycleUsed() override{
            return 1;
        }

        bool isStateFullNode() override{
            return true;
        }

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
        }

    };

}

#endif //KATHRYN_STARTNODE_H

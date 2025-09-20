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

        expression* exitExpr  = nullptr;
        StateReg* _startState = nullptr;
        Operable* _rstSig     = nullptr;

        explicit StartNode(Operable* rstSig):
                Node(START_NODE),
                _startState(new StateReg(false)),
                _rstSig(rstSig){
                assert(_rstSig != nullptr);
                exitExpr = &(*_startState == upState);
                setClockMode(CM_POSEDGE);
        }

        void makeUnsetStateEvent() override{
            assert(_startState != nullptr);
            _startState->makeUnSetStateEvent(getClockMode());
        }

        Operable* getExitOpr() override{
            assert(_rstSig != nullptr);
            assert(exitExpr != nullptr);
            return exitExpr;
        }

        void assign() override{
            _startState->addDependState(_rstSig, nullptr, getClockMode());
            makeUnsetStateEvent();
            _startState->setVarName("startNode");
            exitExpr->setVarName("startExpr");

            /**no need to reset due to it used*/
        }

        int getCycleUsed() override{
            return 1;
        }

    };

}

#endif //KATHRYN_STARTNODE_H

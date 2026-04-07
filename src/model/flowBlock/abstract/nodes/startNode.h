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

        expression* _exit_expr  = nullptr;
        StateReg*   _startState = nullptr;
        Operable*   _rstSig     = nullptr;

        explicit StartNode(Operable* rstSig):
                Node(START_NODE),
                _startState(new StateReg(false)),
                _rstSig    (rstSig){
            assert     (_rstSig != nullptr);
            _exit_expr = &(*_startState == upState);
            set_clock_mode(CM_POSEDGE);
        }

        void make_unset_state_event() override{
            assert(_startState != nullptr);
            _startState->makeUnSetStateEvent(get_clock_mode());
        }

        Operable* get_exit_opr_ptr() override{
            assert(_rstSig != nullptr);
            assert(_exit_expr != nullptr);
            return _exit_expr;
        }

        void assign() override{
            _startState->addDependState(_rstSig, nullptr, get_clock_mode());
            make_unset_state_event();
            _startState->setVarName("startNode");
            _exit_expr->setVarName("startExpr");

            /**no need to reset due to it used*/
        }

        int get_cycle_used() override{
            return 1;
        }

    };

}

#endif //KATHRYN_STARTNODE_H

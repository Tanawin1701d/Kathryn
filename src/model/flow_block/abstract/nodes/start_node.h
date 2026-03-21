//
// Created by tanawin on 6/1/2567.
//

#ifndef KATHRYN_STARTNODE_H
#define KATHRYN_STARTNODE_H

#include "node.h"
#include "model/hw_component/abstract/make_component.h"

namespace kathryn{

    struct StartNode : Node{
        make_val(up_state  , 1, 1);

        expression* exit_expr  = nullptr;
        StateReg* _startState = nullptr;
        Operable* _rstSig     = nullptr;

        explicit StartNode(Operable* rst_sig):
                Node(START_NODE),
                _startState(new StateReg(false)),
                _rstSig(rst_sig){
                assert(_rstSig != nullptr);
                exit_expr = &(*_startState == up_state);
                set_clock_mode(CM_POSEDGE);
        }

        void make_unset_state_event() override{
            assert(_startState != nullptr);
            _startState->make_un_set_state_event(get_clock_mode());
        }

        Operable*get_exit_opr_ptr() override{
            assert(_rstSig != nullptr);
            assert(exit_expr != nullptr);
            return exit_expr;
        }

        void assign() override{
            _startState->add_depend_state(_rstSig, nullptr, get_clock_mode());
            make_unset_state_event();
            _startState->set_var_name("start_node");
            exit_expr->set_var_name("start_expr");

            /**no need to reset due to it used*/
        }

        int get_cycle_used() override{
            return 1;
        }

    };

}

#endif //KATHRYN_STARTNODE_H

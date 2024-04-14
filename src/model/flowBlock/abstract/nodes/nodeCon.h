//
// Created by tanawin on 13/4/2567.
//

#ifndef KATHRYN_NODECON_H
#define KATHRYN_NODECON_H

#include "model/hwComponent/abstract/assignable.h"


namespace kathryn{


    enum CONNECT_NODE_PURPOSE{
        CON_NODE_SET        = 0,
        CON_NODE_RESET_INTR = 1,
        CON_NODE_START_INTR = 2,
        CON_NODE_MAIN_RST   = 3,
        CON_NODE_CNT        = 4
    };


    struct Node;

    struct NODE_META{
        Operable* _condition = nullptr;  ////// index of condition in assignable
        Node*     _state     = nullptr;

        NODE_META(Operable* condition, Node* state):
                _condition(condition),
                _state(state)
        {}
    };



}

#endif //KATHRYN_NODECON_H

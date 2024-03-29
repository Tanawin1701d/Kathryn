//
// Created by tanawin on 27/1/2567.
//

#ifndef KATHRYN_GLOBALCOMPONENT_H
#define KATHRYN_GLOBALCOMPONENT_H

#include "model/hwComponent/wire/wire.h"
#include "model/flowBlock/abstract/nodes/startNode.h"



namespace kathryn{


    extern Wire*      rstWire;
    extern StartNode* startNode;

    void initiateGlobalComponent();

    void resetGlobalComponent();


}

#endif //KATHRYN_GLOBALCOMPONENT_H

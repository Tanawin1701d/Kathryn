//
// Created by tanawin on 27/1/2567.
//

#include "globalComponent.h"


namespace kathryn{


    Wire*      rstWire   = nullptr;
    StartNode* startNode = nullptr;


    void initiateGlobalComponent(){

        rstWire   = &_make<Wire>("rstWire", 1);
        startNode = new StartNode(rstWire);
        startNode->assign();
    }

}
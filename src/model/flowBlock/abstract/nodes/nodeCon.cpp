//
// Created by tanawin on 14/4/2567.
//

#include "node.h"
#include "nodeCon.h"


namespace kathryn{


    void NODE_META::assignNode(Node *nd) {
        assert(_state != nullptr);
        _state = nd->getStateOpr();
        assert(_state != nullptr);
    }
}
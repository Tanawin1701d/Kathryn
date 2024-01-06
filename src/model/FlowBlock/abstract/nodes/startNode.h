//
// Created by tanawin on 6/1/2567.
//

#ifndef KATHRYN_STARTNODE_H
#define KATHRYN_STARTNODE_H

#include "node.h"

namespace kathryn{

    struct StartNode : Node{
        Val upState   = Val(1, "b1");
        Val downState = Val(1, "b0");

        StateReg* _delayReset = nullptr;
        Operable* _rstSig     = nullptr;

        explicit StartNode(Operable* rstSig):
            Node(),
            _delayReset(new StateReg()),
            _rstSig(rstSig){
            assert(_rstSig != nullptr);
        }

        Node* clone() override{
            auto clNode = new StartNode(*this);
            return clNode;
        }

        Operable* getExitOpr() override{
            assert(_rstSig != nullptr);
            return &((*_delayReset == upState) & (*_rstSig == downState));
        }

        void assign() override{
            _delayReset->addDependState(_rstSig, nullptr);
            /***do not make unset state event*/
        }

        int getCycleUsed() override{
            return 1;
        }




    };

}

#endif //KATHRYN_STARTNODE_H

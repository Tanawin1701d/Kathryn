//
// Created by tanawin on 6/1/2567.
//

#ifndef KATHRYN_STARTNODE_H
#define KATHRYN_STARTNODE_H

#include "node.h"
#include "model/hwComponent/abstract/makeComponent.h"

namespace kathryn{

    struct StartNode : Node{
        makeVal(upState, 1, "b1");
        makeVal(downState, 1, "b0");

        StateReg* _delayReset = nullptr;
        Operable* _rstSig     = nullptr;

        explicit StartNode(Operable* rstSig):
            Node(START_NODE),
            _delayReset(new StateReg()),
            _rstSig(rstSig){
            assert(_rstSig != nullptr);
        }

        Node* clone() override{
            auto clNode = new StartNode(*this);
            clNode->setCpyPtr(this);
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

//
// Created by tanawin on 6/1/2567.
//

#ifndef KATHRYN_STARTNODE_H
#define KATHRYN_STARTNODE_H

#include "node.h"
#include "model/hwComponent/abstract/makeComponent.h"

namespace kathryn{

    struct StartNode : Node{
        makeWire(checkWire, 1);

        StateReg* _startState = nullptr;
        Operable* _rstSig     = nullptr;

        explicit StartNode(Operable* rstSig):
                Node(START_NODE),
                _startState(new StateReg(false)),
                _rstSig(rstSig){
                assert(_rstSig != nullptr);
        }

        void makeUnsetStateEvent() override{
            assert(_startState != nullptr);
            _startState->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{
            assert(_rstSig != nullptr);
            return &(checkWire);
        }

        void assign() override{
            ////// start node
            _startState->addDependState(_rstSig, nullptr);
            makeUnsetStateEvent();
            _startState->setVarName("startNode");
            ////// check wire
            checkWire = *_startState;
            checkWire.asBci();
            checkWire.setVarName("startWireRep");

            /**no need to reset due to it used*/
        }

        int getCycleUsed() override{
            return 1;
        }

    };

}

#endif //KATHRYN_STARTNODE_H

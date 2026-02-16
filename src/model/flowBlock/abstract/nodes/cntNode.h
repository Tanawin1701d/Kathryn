//
// Created by tanawin on 14/2/2026.
//

#ifndef MODEL_FLOWBLOCK_ABSTRACT_NODES_CNTNODE_H
#define MODEL_FLOWBLOCK_ABSTRACT_NODES_CNTNODE_H

#include "node.h"
#include "model/flowBlock/abstract/spReg/cntReg.h"

namespace kathryn{

    struct CounterNode : Node{
        CounterReg* _counter = nullptr;

        explicit CounterNode(int lastLoopCnt, CLOCK_MODE clockMode):
                Node(COUNTER_NODE){

            assert(lastLoopCnt > 0);
             _counter = new CounterReg(lastLoopCnt);
            addCycleRelatedReg(_counter);
            setClockMode(clockMode);
        }

        void makeUnsetStateEvent() override{
            assert(false);
        }

        void makeUserResetEvent() override{
            if(isThrereIntReset()){
                _counter->makeUserRstEvent(intReset->getExitOpr(), getClockMode());
            }
        }

        void makeIncCounterEvent(Node* incNode){
            assert(incNode != nullptr);
            _counter->makeIncEvent(incNode->getExitOpr(), getClockMode());
        }

        Operable* getExitOpr() override{
            assert(_counter != nullptr);
            return _counter->generateEndExpr();
        }

        Operable* getCounter(){return _counter;}

        void assign() override{
            assert(_counter!= nullptr);
            /**normal start event*/
            for(auto nodeSrc: nodeSrcs){
                _counter->addDependState(nodeSrc.dependNode->getExitOpr(),
                                         nodeSrc.condition, getClockMode());
            }
            /** unset event*/
            makeUserResetEvent();
            _counter->setVarName(identName);
        }

        int getCycleUsed() override{
            assert(_counter != nullptr);
            ///// hold should not be considered here
            return _counter->getLoopCnt();
        }

    };

}

#endif //MODEL_FLOWBLOCK_ABSTRACT_NODES_CNTNODE_H

//
// Created by tanawin on 12/4/2567.
//

#ifndef KATHRYN_INTERUPTNODE_H
#define KATHRYN_INTERUPTNODE_H

#include "node.h"

namespace kathryn{


    struct InteruptNode : public Node{

        std::vector<Operable*> srcInterrupts;
        Operable* interruptOutput = nullptr;

        explicit InteruptNode(): Node(INTERRUPT_NODE){}


        int getCycleUsed() override{ return 0;}

        Operable* getExitOpr() override{
            assert(interruptOutput != nullptr);
            return interruptOutput;
        }

        void assign() override{
            ////// there should no dependNode
            assert(dependNodes.empty());
            assert(!srcInterrupts.empty());
            /** build interrupt*/
            for (Operable* srcIntOpr: srcInterrupts){
                addLogic(interruptOutput, srcIntOpr, BITWISE_OR);
            }
        }

        void addInterruptSignal(Operable* srcInterrupt){
            assert(srcInterrupt != nullptr);
            srcInterrupts.push_back(srcInterrupt);
        }

        bool isStateFullNode() override{ return false;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(interruptOutput != nullptr);
        }

    };

}

#endif //KATHRYN_INTERUPTNODE_H

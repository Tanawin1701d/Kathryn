//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_WAITNODE_H
#define KATHRYN_WAITNODE_H

#include "node.h"
#include "model/FlowBlock/abstract/spReg/waitReg.h"

namespace kathryn{

    struct WaitCondNode : Node{

        CondWaitStateReg* _condWaitStateReg = nullptr;

        explicit WaitCondNode(Operable* waitCond):
                Node(WAITCOND_NODE){
            assert(waitCond != nullptr);
            _condWaitStateReg = new CondWaitStateReg(waitCond);
        }

        Node* clone() override{
            auto clNode = new WaitCondNode(*this);
            clNode->setCpyPtr(this);
            return clNode;
        }

        void makeUnsetStateEvent() override{
            assert(_condWaitStateReg != nullptr);
            _condWaitStateReg->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{
            assert(_condWaitStateReg != nullptr);
            return _condWaitStateReg->generateEndExpr();
        }

        void assign() override{
            auto dependNodeOpr = transformAllDepNodeToOpr();
            assert(dependNodeOpr != nullptr);
            _condWaitStateReg->addDependState(dependNodeOpr, condition);
            makeUnsetStateEvent();
        }

        int getCycleUsed() override {return -1;}

        void simStartCurCycle() override{
            if (isCurCycleSimulated()){
                return;
            }
            setSimStatus();


        }
    };

    struct WaitCycleNode : Node{
        int _cycle = -1;
        CycleWaitStateReg* _cycleWaitStateReg = nullptr;

        explicit WaitCycleNode(int cycle):
                Node(WAITCYCLE_NODE),
                _cycle(cycle){

            _cycleWaitStateReg = new CycleWaitStateReg(cycle);
        }

        explicit WaitCycleNode(Operable* opr):
                Node(WAITCYCLE_NODE)
        {
            _cycleWaitStateReg = new CycleWaitStateReg(opr);
        }

        Node* clone() override{
            auto clNode = new WaitCycleNode(*this);
            clNode->setCpyPtr(this);
            return clNode;
        }

        void makeUnsetStateEvent() override{
            assert(_cycleWaitStateReg != nullptr);
            _cycleWaitStateReg->makeUnSetStateEvent();
        }

        Operable* getExitOpr() override{
            assert(_cycleWaitStateReg != nullptr);
            return _cycleWaitStateReg->generateEndExpr();
        }

        void assign() override{
            auto dependNodeOpr = transformAllDepNodeToOpr();
            assert(dependNodeOpr != nullptr);
            _cycleWaitStateReg->addDependState(dependNodeOpr, condition);
            makeUnsetStateEvent();
        }

        int getCycleUsed() override {return _cycle;}

    };

}

#endif //KATHRYN_WAITNODE_H

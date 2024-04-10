//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_WAITNODE_H
#define KATHRYN_WAITNODE_H

#include "node.h"
#include "model/flowBlock/abstract/spReg/waitReg.h"

namespace kathryn{

    struct WaitCondNode : Node{

        CondWaitStateReg* _condWaitStateReg = nullptr;

        explicit WaitCondNode(Operable* waitCond):
                Node(WAITCOND_NODE){
            assert(waitCond != nullptr);
            _condWaitStateReg = new CondWaitStateReg(waitCond);
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
            _condWaitStateReg->setVarName(identName);
        }

        int getCycleUsed() override {return -1;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            bool shouldIncStat = _condWaitStateReg->isSimAtWaiting();
            if (shouldIncStat){
                setBlockOrNodeRunning();
                incEngine();
            }



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
            _cycleWaitStateReg->setVarName(identName);
        }

        int getCycleUsed() override {return _cycle;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            bool shouldIncStat =  _cycleWaitStateReg->isSimAtWaiting();
            if (shouldIncStat){
                incEngine();
                setBlockOrNodeRunning();
            }



        }

    };

}

#endif //KATHRYN_WAITNODE_H

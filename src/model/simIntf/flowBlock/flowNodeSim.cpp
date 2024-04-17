//
// Created by tanawin on 17/4/2567.
//

#include "flowNodeSim.h"
#include "model/flowBlock/abstract/nodes/node.h"


namespace kathryn{


    FlowNodeSimEngine::FlowNodeSimEngine(Node* master):
    FlowBaseSimEngine(),
    _master(master)
    {}

    void FlowNodeSimEngine::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isNodeRunning = false;
        for (CtrlFlowRegBase* creg: _master->relatedCycleConsumeReg){
            isNodeRunning |= creg->getRtlValItf()->getCurVal().getLogicalValue();
        }
        if (isNodeRunning){
            setBlockOrNodeRunning();
            incEngine();
        }
    }

    void FlowNodeSimEngine::simExitCurCycle() {
        unsetBlockOrNodeRunning();
        unSetSimStatus();
    }
}
//
// Created by tanawin on 18/2/2567.
//

#include "MemBlock.h"

namespace kathryn{


    MemBlock::MemBlock(ull depth, int width):
            Identifiable(TYPE_MEM_BLOCK),
            HwCompControllerItf(false),
            MemSimulatable(depth, width),
            ModelDebuggable(),
            DEPTH_SIZE(depth),
            WIDTH_SIZE(width)
    {
        assert(width > 0);
    }

    MemBlockEleHolder& MemBlock::operator[](const Operable& indexer) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, &indexer);
        memBlockAgents.push_back(memBlockEleHolder);
        return *memBlockEleHolder;
    }

    MemBlockEleHolder& MemBlock::operator[](const int idx) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, idx);
        memBlockAgents.push_back(memBlockEleHolder);
        return *memBlockEleHolder;
    }

    void MemBlock::simStartCurCycle(){
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        for (auto agentHolder: memBlockAgents){
            agentHolder->simStartCurCycle();
        }
    }

    void MemBlock::simStartNextCycle(){
        if (isNextValSim()){
            return;
        }
        setNextValSimStatus();
        for (auto agentHolder: memBlockAgents){
            agentHolder->simStartNextCycle();
        }
    }

    void MemBlock::simExitCurCycle(){
        for (auto agentHolder: memBlockAgents){
            agentHolder->simExitCurCycle();
        }
    }



}

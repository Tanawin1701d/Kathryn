//
// Created by tanawin on 18/2/2567.
//

#include "MemBlock.h"
#include "model/controller/controller.h"

namespace kathryn{


    MemBlock::MemBlock(ull depth, int width):
            Identifiable(TYPE_MEM_BLOCK),
            HwCompControllerItf(false),
            MemSimEngine(depth, width),
            ModelDebuggable(),
            DEPTH_SIZE(depth),
            WIDTH_SIZE(width)
    {
        assert(width > 0);
        com_init();
    }

    void MemBlock::com_init(){
        ctrl->on_memBlk_init(this);
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
        /////std::cout << "start CurMemBlk PreDeclar" << std::endl;
        if (isCurValSim()){
            return;
        }

        setCurValSimStatus();
        for (auto agentHolder: memBlockAgents){
            ////std::cout << "start CurMemBlk" << std::endl;
            agentHolder->getSimEngine()->simStartCurCycle();
        }
    }

    void MemBlock::simStartNextCycle(){
        assert(!isNextValSim());
        setNextValSimStatus(); /// it only can be invoked one;
        ////std::cout << "start Next MemBlk PreDeclar" << std::endl;
        for (auto agentHolder: memBlockAgents){
            ////// we will sim only write mode else is not
            if (!agentHolder->isReadMode()){
                agentHolder->getSimEngine()->simStartNextCycle();
            }
        }
    }

    void MemBlock::simExitCurCycle(){
        _isCurValSim = false;
        _isNextValSim = false;
        for (auto agentHolder: memBlockAgents){
            agentHolder->getSimEngine()->simExitCurCycle();
        }

//        std::cout <<"---------mem regiion" << std::endl;
//        for (int i = 0; i < DEPTH_SIZE; i++){
//            std::cout << memBlk[i].getBiStr() << std::endl;
//        }
    }



}

//
// Created by tanawin on 22/11/2024.
//

#include "mtModule.h"

namespace kathryn{


    void MtModule::auxFlow() {
        mtAgent.flow();
    }

    void MtModule::augmentModule() {



    }


    //////////// augment section

    void MtModule::augRegRcvScp(std::vector<Reg*>& regs){
        for(Reg* regPtr: regs){
            augRegRcvScp(regPtr);
        }
    }


    void MtModule::augMemsRcvScp(std::vector<MemBlock*>& memBlks){
        for(MemBlock* memBlockPtr: memBlks){
            augMemsRcvScp(memBlockPtr);
        }
    }


}
//
// Created by tanawin on 20/6/2024.
//

#include "memGen.h"

namespace kathryn{

    MemGen::MemGen(ModuleGen* mdGenMaster,
                        logicLocalCef cerf,
                        MemBlock* memBlockMaster):
    LogicGenBase(
        mdGenMaster,
        cerf,
        (Assignable*) memBlockMaster,
        (Identifiable*) memBlockMaster
    ),
    _master(memBlockMaster){}

    std::string MemGen::decVariable(){
        int memWidth = _master->getWidthSize();
        int memDepth = _master->getDepthSize();
        return "reg [" + std::to_string(memWidth-1) +
               ": 0] " + getOpr() + " [" +
               std::to_string(memDepth-1) + ": 0];";
    }



}
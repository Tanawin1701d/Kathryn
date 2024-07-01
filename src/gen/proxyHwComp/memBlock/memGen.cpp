//
// Created by tanawin on 20/6/2024.
//

#include "memGen.h"
#include "model/hwComponent/memBlock/MemBlock.h"

namespace kathryn{

    MemGen::MemGen(ModuleGen* mdGenMaster,
                        MemBlock* memBlockMaster):
    LogicGenBase(
        mdGenMaster,
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


    bool MemGen::compare(LogicGenBase* lgb){
        assert(lgb->getLogicCef().comptype == TYPE_MEM_BLOCK);

        auto* rhs = dynamic_cast<MemGen*>(lgb);
        ////// check depth only; do not check width because cerf handle it
        return checkCerfEqLocally(rhs->_cerf) &&
            (_master->getDepthSize() == rhs->_master->getDepthSize());
    }



}
//
// Created by tanawin on 20/6/2024.
//

#include "memGen.h"
#include "model/hwComponent/memBlock/memBlock.h"

namespace kathryn{

    MemGen::MemGen(ModuleGen* mdGenMaster,
                        MemBlock* memBlockMaster):
    LogicGenBase(
        mdGenMaster,
        nullptr,
        (Identifiable*) memBlockMaster
    ),
    _master(memBlockMaster){}


    std::string MemGen::decVariable(){
        int memWidth = _master->getWidthSize();
        int memDepth = _master->getDepthSize();
        return "reg [" + std::toString(memWidth-1) +
               ": 0] " + LogicGenBase::getOpr() + " [" +
               std::toString(memDepth-1) + ": 0];";
    }

    std::string MemGen::getOpr(Slice sl){
        assert( (sl.start == 0) &&
                (sl.getSize() == _master->getDepthSize()));

        return LogicGenBase::getOpr();
    }



}
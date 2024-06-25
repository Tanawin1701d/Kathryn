//
// Created by tanawin on 20/6/2024.
//

#include "nestGen.h"

#include "gen/proxyHwComp/module/moduleGen.h"


namespace kathryn{

    NestGen::NestGen(ModuleGen*    mdGenMaster,
                     logicLocalCef cerf,
                     nest*         nestMaster):
    LogicGenBase(mdGenMaster,
                 cerf,
                 (Assignable*) nestMaster,
                 (Identifiable*) nestMaster),
    _master(nestMaster){
        assert(nestMaster != nullptr);
    }

    std::string NestGen::decIo(){
        assert(false);
    }

    std::string NestGen::decVariable(){
        Slice sl = _master->getOperableSlice();

        return "wire [" + std::to_string(sl.stop-1) +
            ": 0] " + getOpr() + ";";
    }

    std::string NestGen::decOp(){
        std::string preRetStr = "assign " + getOpr() + " = {";

        bool isFirst = true;

        for (auto iter = _routedNestList.rbegin();
                  iter != _routedNestList.rend();
                  iter++
        ){
            if(!isFirst){
                preRetStr += ",";
            }
            preRetStr += getOprStrFromOpr(*iter);
            isFirst = false;
        }

        preRetStr += "};";
        return preRetStr;
    }

}

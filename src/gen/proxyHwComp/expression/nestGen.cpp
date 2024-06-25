//
// Created by tanawin on 20/6/2024.
//

#include "nestGen.h"

#include <utility>
#include <gen/proxyHwComp/module/moduleGen.h>

#include "model/hwComponent/expression/nest.h"


namespace kathryn{

    NestGen::NestGen(ModuleGen*    mdGenMaster,
                     logicLocalCef cerf,
                     nest*         nestMaster):
    LogicGenBase(mdGenMaster,
                 std::move(cerf),
                 (Assignable*) nestMaster,
                 (Identifiable*) nestMaster),
    _master(nestMaster){
        assert(nestMaster != nullptr);
    }

    void NestGen::routeDep(){
        std::vector<NestMeta> nestList = _master->getNestList();
        for (NestMeta& ele: nestList){
            assert(ele.opr != nullptr);
            Operable* routedOpr = _mdGenMaster->routeSrcOprToThisModule(ele.opr);
            _routedNestList.push_back(routedOpr);
        }
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

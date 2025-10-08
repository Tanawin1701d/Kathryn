//
// Created by tanawin on 20/6/2024.
//

#include "nestGen.h"

#include <utility>
#include <gen/proxyHwComp/module/moduleGen.h>

#include "model/hwComponent/expression/nest.h"


namespace kathryn{

    NestGen::NestGen(ModuleGen*    mdGenMaster,
                     nest*         nestMaster):
    LogicGenBase(mdGenMaster,
                 (Assignable*) nestMaster,
                 (Identifiable*) nestMaster),
    _master(nestMaster){
        assert(nestMaster != nullptr);
    }

    void NestGen::routeDep(){
        std::vector<NestMeta> nestList = _master->getNestList();
        for (NestMeta& ele: nestList){
            assert(ele.opr1 != nullptr);
            Operable* routedOpr = _mdGenMaster->routeSrcOprToThisModule(ele.opr1);
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

    bool NestGen::compare(LogicGenBase* lgb){
        assert(lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_NEST);
        auto* rhs      = dynamic_cast<NestGen*>(lgb);
        bool cerfCheck = checkCerfEqLocally(*rhs);
        ///////// check cerf and nest list size first
        if ( !(cerfCheck &&
               (_routedNestList.size() == rhs->_routedNestList.size())
               )){ return false;}
        //////// check src nest list is equal
        bool isEqual = true;
        for (int idx = 0; idx < _routedNestList.size(); idx++){
            isEqual &=
                cmpEachOpr(_routedNestList[idx], rhs->_routedNestList[idx],
                getModuleGen(), rhs->getModuleGen(), SUBMOD
                );
        }
        return isEqual;
    }


}

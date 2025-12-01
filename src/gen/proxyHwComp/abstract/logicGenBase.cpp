//
// Created by tanawin on 20/6/2024.
//

#include "logicGenBase.h"

#include <utility>
#include "gen/proxyHwComp/module/moduleGen.h"

namespace kathryn{


    LogicGenBase::LogicGenBase(ModuleGen*    mdGenMaster,
                               Assignable*   asb,
                               Identifiable* ident):
_mdGenMaster(mdGenMaster),
_asb(asb),
_ident(ident){
    assert(mdGenMaster != nullptr);
}

std::string LogicGenBase::getOprStrFromOpr(Operable* opr1){
    assert(opr1 != nullptr);
    return opr1->getExactOperable().
        getLogicGenBase()->getOpr(opr1->getOperableSlice());
}

std::string LogicGenBase::getOprStrFromOprAndShinkMsb(Operable* opr1, int targetSize){

    int srcSize = opr1->getOperableSlice().getSize();
    assert(targetSize <= srcSize);
    assert(targetSize > 0);
    std::string pureSrcStr = getOprStrFromOpr(opr1);

    if (targetSize == srcSize){
        return pureSrcStr;
    }
    return pureSrcStr + "[" + std::to_string(targetSize-1) + ": 0]";

}

std::string LogicGenBase::getOpr(){
    assert(_ident != nullptr);
    // if (!_ident->isUserVar()){
    //     return _ident->getGlobalName();
    // }
    return _ident->getGlobalName() + "_" + _ident->getVarName();

}

std::string LogicGenBase::getOpr(Slice sl){
        assert(sl.checkValidSlice());
        if (sl == _asb->getAssignSlice()){
            return getOpr();
        }
        return getOpr() + "[" + std::to_string(sl.stop-1) +
               ": " + std::to_string(sl.start) + "]";
}
}

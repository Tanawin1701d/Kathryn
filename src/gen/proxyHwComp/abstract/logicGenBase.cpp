//
// Created by tanawin on 20/6/2024.
//

#include "logicGenBase.h"

#include <utility>
#include "gen/proxyHwComp/module/moduleGen.h"

namespace kathryn{


    LogicGenBase::LogicGenBase(ModuleGen*    mdGenMaster,
                               logicLocalCef cerf,
                               Assignable*   asb,
                               Identifiable* ident):
_mdGenMaster(mdGenMaster),
_cerf(std::move(cerf)),
_asb(asb),
_ident(ident){
    assert(mdGenMaster != nullptr);
}

std::string LogicGenBase::getOprStrFromOpr(Operable* opr){
    assert(opr != nullptr);
    return opr->getExactOperable().
        getLogicGenBase()->getOpr(opr->getOperableSlice());
}

std::string LogicGenBase::getOpr(){
    assert(_ident != nullptr);
    if (!_ident->isUserVar()){
        return _ident->getGlobalName();
    }
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

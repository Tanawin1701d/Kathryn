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
_cerf({}),
_asb(asb),
_ident(ident){
    assert(mdGenMaster != nullptr);
}

std::string LogicGenBase::getOprStrFromOpr(Operable* opr){
    assert(opr != nullptr);
    return opr->getExactOperable().
        getLogicGenBase()->getOpr(opr->getOperableSlice());
}

void LogicGenBase::genCerf(MODULE_GEN_GRP mgg,int grpIdx, int idx){
        assert(grpIdx >= 0);
        assert(idx    >= 0);
        assert(_asb   != nullptr);
        assert(_ident != nullptr);
        _cerf.comptype   = _ident->getType();
        _cerf.md_gen_grp = mgg;
        _cerf.varMeta    = _ident->getVarMeta();
        _cerf.grpIdx     = grpIdx;
        _cerf.idx        = idx;
        _cerf.curSl      = _asb->getAssignSlice();
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


bool LogicGenBase::checkCerfEqLocally(const logicLocalCef& rhsCerf){
        bool ans = true;
        ans &= _cerf.comptype          == rhsCerf.comptype;
        ans &= _cerf.md_gen_grp        == rhsCerf.md_gen_grp;
        ans &= _cerf.varMeta.varType  == rhsCerf.varMeta.varType;
        ans &= _cerf.varMeta.isUser   == rhsCerf.varMeta.isUser;
        ans &= _cerf.grpIdx           == rhsCerf.grpIdx;
        ans &= _cerf.idx              == rhsCerf.idx;
        ans &= _cerf.curSl            == rhsCerf.curSl;
        return ans;
}

bool LogicGenBase::cmpEachOpr(Operable* srcA, Operable* srcB,
                              ModuleGen* srcMdA, ModuleGen* srcMdB){

        bool isANull = srcA == nullptr;
        bool isBNull = srcB == nullptr;

        if (isANull | isBNull){
            return isANull == isBNull; ///// if one is not null and one is null escape it
        }


        bool isAStayInMd = srcA->getLogicGenBase()->getModuleGen() == srcMdA;
        bool isBStayInMd = srcB->getLogicGenBase()->getModuleGen() == srcMdB;

        if (isAStayInMd  ^ isBStayInMd){
            return false; //// not stay in the same module but one is
        }

        if (isAStayInMd){
            return srcA->getLogicGenBase()->
                   checkCerfEqLocally(srcB->getLogicGenBase()->_cerf);
        }

        ///////// we must check cerf of its master module

        bool sameLogicCerf = srcA->getLogicGenBase()->
                              checkCerfEqLocally(srcB->getLogicGenBase()->_cerf);

        bool sameMdCerf = false;
        return sameLogicCerf & sameMdCerf;


    }










}

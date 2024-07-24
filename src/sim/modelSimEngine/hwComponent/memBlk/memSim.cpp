//
// Created by tanawin on 31/5/2024.
//

#include "memSim.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "sim/controller/simController.h"



namespace kathryn{
    MemSimEngine::MemSimEngine(MemBlock* master):
        _master(master){ assert(master != nullptr); }

    std::string MemSimEngine::getVarName(){ return _master->getGlobalName(); }

    std::vector<std::string> MemSimEngine::getRegisVarName(){
        return {getVarName()};
    }


    ull MemSimEngine::getVarId(){ return _master->getGlobalId(); }

    SIM_VALREP_TYPE MemSimEngine::getValR_Type(){
        return getMatchSVT(_master->getWidthSize());
    }


    void MemSimEngine::createGlobalVariable(CbBaseCxx& cb){
        std::string sizeStr = std::to_string(_master->getDepthSize());
        SIM_VALREP_TYPE svt     = getValR_Type();
        std::string     typeStr = SVT_toType(svt);
        cb.addSt(typeStr + " " + getVarName() + "[" + sizeStr + "]");
    }

    void MemSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getVal(getVarName());
        proxyRep.setSize(_master->getWidthSize());
    }

    ValRepBase& MemSimEngine::getProxyRep(){
        assert(proxyRep.isInUsed());
        return proxyRep;
    }
}

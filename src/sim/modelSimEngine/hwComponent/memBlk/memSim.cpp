//
// Created by tanawin on 31/5/2024.
//

#include "memSim.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "sim/controller/simController.h"



namespace kathryn{
    MemSimEngine::MemSimEngine(MemBlock* master):
        _master(master){ assert(master != nullptr); }

    ValR MemSimEngine::getValRep(){
        std::string     name = _master->getGlobalName();
        int             size = _master->getWidthSize();
        SIM_VALREP_TYPE svt  = getMatchSVT(size);
        return {svt, size, name};
    }

    std::vector<std::string> MemSimEngine::getRegisVarName(){
        return {getValRep().getData()};
    }


    ull MemSimEngine::getVarId(){ return _master->getGlobalId(); }

    SIM_VALREP_TYPE MemSimEngine::getValR_Type(){
        return getMatchSVT(_master->getWidthSize());
    }


    void MemSimEngine::createGlobalVariable(CbBaseCxx& cb){
        ull depth = _master->getDepthSize();
        cb.addSt(getValRep().buildMemVar(depth));
    }

    void MemSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getVal(getValRep().getData());
        proxyRep.setSize(_master->getWidthSize());
    }

    ValRepBase& MemSimEngine::getProxyRep(){
        assert(proxyRep.isInUsed());
        return proxyRep;
    }
}

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

    void MemSimEngine::createGlobalVariable(CbBaseCxx& cb){
        std::string sizeStr = std::to_string(_master->getDepthSize());
        cb.addSt("ull "+ getVarName() + "[" + sizeStr + "]");
    }

    void MemSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = new ValRepBase(
            _master->getWidthSize(),
            *modelSimEvent->getVal(getVarName()));
    }

    ValRepBase* MemSimEngine::getProxyRep(){
        assert(proxyRep != nullptr);
        return proxyRep;
    }
}

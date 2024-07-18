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

    std::string MemSimEngine::createGlobalVariable(){
        return "ull "
            + getVarName() +
            "[" + std::to_string(_master->getDepthSize()) + "];\n";
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

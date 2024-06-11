//
// Created by tanawin on 31/5/2024.
//

#include "memSimEngine.h"
#include "model/simIntf/base/proxyEventBase.h"
#include "sim/controller/simController.h"
#include "model/hwComponent/memBlock/MemBlock.h"


namespace kathryn{


    MemSimEngine::MemSimEngine(MemBlock* master):
    _master(master)
    { assert(master != nullptr);}

    std::string MemSimEngine::getVarName(){ return _master->getGlobalName();}

    std::vector<std::string> MemSimEngine::getRegisVarName(){
        return {getVarName()};
    }


    ull         MemSimEngine::getVarId(){return _master->getGlobalId();}

    std::string MemSimEngine::createVariable(){
        return "ValRep <" + std::to_string(_master->getWidthSize()) + ">"
            + getVarName() +
            "["+ std::to_string(_master->getDepthSize()) + "];\n";
    }

    void MemSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getValRep(getVarName());
    }

    ValRepBase* MemSimEngine::getProxyRep(){
        assert(proxyRep != nullptr);
        return proxyRep;
    }

}

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

        return {SIM_VALREP_TYPE_ALL(size), size, name};
    }

    std::vector<std::string> MemSimEngine::getRegisVarName(){
        return {getValRep().getData()};
    }


    ull MemSimEngine::getVarId(){ return _master->getGlobalId(); }

    SIM_VALREP_TYPE_ALL MemSimEngine::getValR_Type(){
        return SIM_VALREP_TYPE_ALL(_master->getWidthSize());
    }

    void MemSimEngine::markSV(const std::string& str){
        markSV_base(str);
    }

    void MemSimEngine::createGlobalVariable(CbBaseCxx& cb){
        ull depth = _master->getDepthSize();
        cb.addSt(getValRep().buildMemVar(depth));
    }

    void MemSimEngine::createUserMarkValue(CbBaseCxx& cb){
        if (isMarkSV){
            cb.addSt(getValRep().buildMemVarPtr(markSV_key));
        }
    }


    ///////   return value

    void MemSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getVal(getValRep().getData());
        proxyRep.setSize(_master->getWidthSize());
        if (getValR_Type().type == SVT_U64M){
            assert(getValR_Type().subType > 0);
            proxyRep.setContinLength(getValR_Type().subType);
        }
    }

    ValRepBase& MemSimEngine::getProxyRep(){
        assert(proxyRep.isInUsed());
        return proxyRep;
    }
}

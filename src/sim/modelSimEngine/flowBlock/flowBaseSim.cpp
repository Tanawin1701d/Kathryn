//
// Created by tanawin on 17/4/2567.
//

#include "flowBaseSim.h"

#include "sim/controller/simController.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "sim/modelSimEngine/base/proxyEventBase.h"
#include "util/str/strUtil.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"


namespace kathryn{
    FlowBaseSimEngine::FlowBaseSimEngine(FlowBlockBase* flowBlockBase):
        _flowBlockBase(flowBlockBase),
        _writer(nullptr){
        assert(_flowBlockBase != nullptr);
    }

    std::string FlowBaseSimEngine::getVarName(){
        return "PERF_" + _flowBlockBase->getGlobalName();
    }

    std::string FlowBaseSimEngine::getVarNameCurStatus(){
        return getVarName() + "_CURBIT";
    }

    std::vector<std::string> FlowBaseSimEngine::getRegisVarName(){
        return {getVarName(), getVarNameCurStatus()};
    }

    ull FlowBaseSimEngine::getVarId(){
        return _flowBlockBase->getGlobalId();
    }

    SIM_VALREP_TYPE FlowBaseSimEngine::getValR_Type(){
        return SVT_U64;
    }


    void FlowBaseSimEngine::getRecurVarName(std::vector<std::string>& result){
        result.push_back(getVarName());
        for (FlowBlockBase* fb : _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            subBlockSimEngine->getRecurVarName(result);
        }
        for (FlowBlockBase* fb : _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            conBlockSimEngine->getRecurVarName(result);
        }
    }

    void FlowBaseSimEngine::getRecurVarNameCurStsatus(std::vector<std::string>& result){
        result.push_back(getVarNameCurStatus());
        for (FlowBlockBase* fb : _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            subBlockSimEngine->getRecurVarNameCurStsatus(result);
        }
        for (FlowBlockBase* fb : _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            conBlockSimEngine->getRecurVarNameCurStsatus(result);
        }
    }


    void FlowBaseSimEngine::createGlobalVariable(CbBaseCxx& cb){
        SIM_VALREP_TYPE svt = getValR_Type();
        std::string typeStr = SVT_toType(svt);
        std::string typeSingleBitStr = SVT_toType(SVT_U8);

        cb.addSt(typeStr + " " + getVarName() + " = 0");
        cb.addSt(typeSingleBitStr + " " + getVarNameCurStatus() + " = 0");


        for (FlowBlockBase* fb : _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            subBlockSimEngine->createGlobalVariable(cb);
        }
        for (FlowBlockBase* fb : _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            conBlockSimEngine->createGlobalVariable(cb);
        }
    }

    void FlowBaseSimEngine::createOp(CbBaseCxx& cb){

        cb.addCm("////////////////////////////////////");
        cb.addCm(_flowBlockBase->getGlobalName());
        ////////////////////////////////////////////////////////////////////////////
        //////////// subBlock build
        ////////////////////////////////////////////////////////////////////////////
        for (FlowBlockBase* fb : _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            subBlockSimEngine->createOp(cb.addSubBlock());
        }

        /////////////////////////////////////////////////////////////////////////////
        ///////////// this block purpose
        /////////////////////////////////////////////////////////////////////////////
        cb.addSt(getVarNameCurStatus() + " = 0");

        //////////// basic node recruitment
        cb.addCm("basic node rc");
        for (Node* sysNode : _flowBlockBase->getSysNodes()){
            assert(sysNode != nullptr);
            for (CtrlFlowRegBase* stateReg : sysNode->getCycleRelatedReg()){
                if (stateReg != nullptr){
                    std::string regName = stateReg->getSimEngine()->getVarName();
                    cb.addSt( getVarNameCurStatus() + " |= " + regName);
                }
            }
        }
        ///////////// sub block recruitment
        for (FlowBlockBase* fb : _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            cb.addSt( getVarNameCurStatus() + " |= "
                + subBlockSimEngine->getVarNameCurStatus());
        }
        cb.addSt(getVarName() + " += " + getVarNameCurStatus());


        ////////////////////////////////////////////////////////////////////////////
        /////////////// conblock block purpose
        ////////////////////////////////////////////////////////////////////////////

        //////////// do for con block

        for (FlowBlockBase* fb : _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            conBlockSimEngine->createOp(cb);
        }

        cb.addCm("////////////////////////////////////");
    }

    //////////////////// return initiate
    ///
    void FlowBaseSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getValPerf(getVarName());
        proxyRep.setSize(bitSizeOfUll);

        _proxyRepCurBit = modelSimEvent->getValPerf(getVarNameCurStatus());
        _proxyRepCurBit.setSize(1);
        ///////// subblock init
        for (FlowBlockBase* subBlock : _flowBlockBase->getSubBlocks()){
            subBlock->getSimEngine()->proxyRetInit(modelSimEvent);
        }
        ///////// conblock init
        for (FlowBlockBase* conBlock : _flowBlockBase->getConBlocks()){
            conBlock->getSimEngine()->proxyBuildInit();
        }
    }

    ValRepBase& FlowBaseSimEngine::getProxyRep(){
        return proxyRep;
    }

    bool FlowBaseSimEngine::isBlockRunning(){
        assert(_proxyRepCurBit.isInUsed());
        return _proxyRepCurBit.getVal();
    }
}

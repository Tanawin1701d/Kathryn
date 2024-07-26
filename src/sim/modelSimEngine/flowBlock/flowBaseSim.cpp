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

    ValR FlowBaseSimEngine::getValRep(){
        return {SIM_VALREP_TYPE_ALL(bitSizeOfUll),
            bitSizeOfUll,
            "PERF_" + _flowBlockBase->getGlobalName()};
    }

    ValR FlowBaseSimEngine::getVarNameCurStatus(){
        ValR base = getValRep();
        return {SIM_VALREP_TYPE_ALL(bitSizeOfUll), 1, base.getData()+"_CURBIT"};
    }

    std::vector<std::string> FlowBaseSimEngine::getRegisVarName(){
        return {getValRep().getData(), getVarNameCurStatus().getData()};
    }

    ull FlowBaseSimEngine::getVarId(){
        return _flowBlockBase->getGlobalId();
    }

    SIM_VALREP_TYPE_ALL FlowBaseSimEngine::getValR_Type(){
        return SIM_VALREP_TYPE_ALL(bitSizeOfUll);
    }


    void FlowBaseSimEngine::getRecurVarName(std::vector<std::string>& result){
        result.push_back(getValRep().getData());
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
        result.push_back(getVarNameCurStatus().getData());
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

        cb.addSt(getValRep().buildVar(0));
        cb.addSt(getVarNameCurStatus().buildVar(0));


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
        cb.addSt(getVarNameCurStatus().eq(ValR(SIM_VALREP_TYPE_ALL(1), 1, "0")).toString());

        //////////// basic node recruitment
        cb.addCm("basic node rc");
        for (Node* sysNode : _flowBlockBase->getSysNodes()){
            assert(sysNode != nullptr);
            for (CtrlFlowRegBase* stateReg : sysNode->getCycleRelatedReg()){
                if (stateReg != nullptr){
                    ValR stateRegRep = stateReg->getSimEngine()->getValRep();
                    cb.addSt( getVarNameCurStatus()
                        .eq(getVarNameCurStatus() | stateRegRep)
                        .toString());
                }
            }
        }
        ///////////// sub block recruitment
        for (FlowBlockBase* fb : _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            cb.addSt( getVarNameCurStatus()
                .eq(getVarNameCurStatus()| subBlockSimEngine->getVarNameCurStatus())
                .toString()
            );

        }
        cb.addSt(
            (getValRep()+
                 getVarNameCurStatus().cast(SIM_VALREP_TYPE_ALL(bitSizeOfUll), bitSizeOfUll)
                 ).toString());


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
        proxyRep = modelSimEvent->getValPerf(getValRep().getData());
        proxyRep.setSize(bitSizeOfUll);

        _proxyRepCurBit = modelSimEvent->getValPerf(getVarNameCurStatus().getData());
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

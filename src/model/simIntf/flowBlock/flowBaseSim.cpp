//
// Created by tanawin on 17/4/2567.
//

#include "flowBaseSim.h"

#include <sim/controller/simController.h>

#include "modelCompile/proxyEventBase.h"

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




    ull FlowBaseSimEngine::getVarId(){
        return _flowBlockBase->getGlobalId();
    }

    std::string FlowBaseSimEngine::createVariable(){
        return "ValRep<64> " + getVarName() + " = 0;\n" +
               "ValRep<1> " + getVarNameCurStatus() +
               " = 0;\n";
    }

    std::string FlowBaseSimEngine::createOp(){

        std::string preRet = "      { ////" + _flowBlockBase->getGlobalName() + "\n";

        //////////// subBlock build
        ///
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            preRet += subBlockSimEngine->createOp();
        }

        //////////// basic node recruitment
        for (Node* sysNode: _flowBlockBase->getSysNodes()){
            assert(sysNode != nullptr);
            for (CtrlFlowRegBase* stateReg: sysNode->getCycleRelatedReg()){
                if (stateReg != nullptr){
                    std::string regName = stateReg->getSimEngine()->getVarName();
                    preRet += "         ";
                    preRet += getVarNameCurStatus() + ".getVal() |= " + regName + ".getLogicValue();\n";
                }
            }
        }
        ///////////// sub block recruitment
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            preRet += "         ";
            preRet += getVarNameCurStatus() + ".getVal() |= "
                   + subBlockSimEngine->getVarNameCurStatus() + ".getVal();\n";
        }
        preRet += "     ";
        preRet += getVarName()          + ".getVal() += " +
                  getVarNameCurStatus() + ".getLogicValue()\n";

        preRet += "     }\n";


        //////////// do for con block
        for (FlowBlockBase* fb: _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            preRet += conBlockSimEngine->createOp();
        }

        return preRet;
    }

    //////////////////// return initiate
    ///
    void FlowBaseSimEngine::proxyRetInit(){
        ProxySimEventBase* proxySimEvent = getSimController()->getProxySimEventPtr();
        proxyRep = proxySimEvent->getValRepPerf(getVarName());
        ///////// subblock init
        for (FlowBlockBase* subBlock: _flowBlockBase->getSubBlocks()){
            subBlock->getSimEngine()->proxyRetInit();
        }
        ///////// conblock init
        for (FlowBlockBase* conBlock: _flowBlockBase->getConBlocks()){
            conBlock->getSimEngine()->proxyBuildInit();
        }
    }

    ValRepBase* FlowBaseSimEngine::getProxyRep(){
        assert(proxyRep != nullptr);
        return proxyRep;
    }







}
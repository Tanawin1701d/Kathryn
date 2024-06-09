//
// Created by tanawin on 17/4/2567.
//

#include "flowBaseSim.h"

#include "sim/controller/simController.h"

#include "util/str/strUtil.h"
#include "model/simIntf/base/proxyEventBase.h"

#include "model/flowBlock/abstract/flowBlock_Base.h"


namespace kathryn{

    FlowBaseSimEngine::FlowBaseSimEngine(FlowBlockBase* flowBlockBase):
    _flowBlockBase(flowBlockBase),
    _writer(nullptr),
    _opSpace(0){
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

    void FlowBaseSimEngine::getRecurVarName(std::vector<std::string>& result){
        result.push_back(getVarName());
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            subBlockSimEngine->getRecurVarName(result);
        }
        for (FlowBlockBase* fb: _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            conBlockSimEngine->getRecurVarName(result);
        }
    }

    void FlowBaseSimEngine::getRecurVarNameCurStsatus(std::vector<std::string>& result){
        result.push_back(getVarName());
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            subBlockSimEngine->getRecurVarNameCurStsatus(result);
        }
        for (FlowBlockBase* fb: _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            conBlockSimEngine->getRecurVarNameCurStsatus(result);

        }
    }


    std::string FlowBaseSimEngine::createVariable(){
        std::string ret = "ValRep<64> " + getVarName() + " = 0;" +
               "ValRep<1> " + getVarNameCurStatus() + " = 0;\n";
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            ret += subBlockSimEngine->createVariable();
        }
        for (FlowBlockBase* fb: _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            ret += conBlockSimEngine->createVariable();
        }
        return ret;
    }

    std::string FlowBaseSimEngine::createOp(){

        std::string space = genConString(' ', _opSpace);

        std::string preRet =  space + "{ ////" + _flowBlockBase->getGlobalName() + "\n";

        //////////// subBlock build
        ///
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            subBlockSimEngine->setOpSpace(_opSpace + SUB_FLOWBLOCK_GEN_OP_SPACE);
            preRet += subBlockSimEngine->createOp();
        }

        //////////// basic node recruitment
        for (Node* sysNode: _flowBlockBase->getSysNodes()){
            assert(sysNode != nullptr);
            for (CtrlFlowRegBase* stateReg: sysNode->getCycleRelatedReg()){
                if (stateReg != nullptr){
                    std::string regName = stateReg->getSimEngine()->getVarName();
                    preRet += space;
                    preRet += getVarNameCurStatus() + ".getRefVal() |= " + regName + ".getLogicValue();\n";
                }
            }
        }
        ///////////// sub block recruitment
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            preRet += space;
            preRet += getVarNameCurStatus() + ".getRefVal() |= "
                   + subBlockSimEngine->getVarNameCurStatus() + ".getVal();\n";
        }
        preRet += space;
        preRet += getVarName()          + ".getRefVal() += " +
                  getVarNameCurStatus() + ".getLogicValue();\n";

        preRet +=  space + "}\n";


        //////////// do for con block
        for (FlowBlockBase* fb: _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            conBlockSimEngine->setOpSpace(_opSpace);
            preRet += conBlockSimEngine->createOp();
        }

        return preRet;
    }

    //////////////////// return initiate
    ///
    void FlowBaseSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getValRepPerf(getVarName());
        ///////// subblock init
        for (FlowBlockBase* subBlock: _flowBlockBase->getSubBlocks()){
            subBlock->getSimEngine()->proxyRetInit(modelSimEvent);
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

    bool FlowBaseSimEngine::isBlockRunning(){
        assert(proxyRep != nullptr);
        return proxyRep->getVal();
    }







}
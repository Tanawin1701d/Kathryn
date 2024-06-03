//
// Created by tanawin on 17/4/2567.
//

#include "flowBaseSim.h"
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

        std::string preRet = "      { ////" + _flowBlockBase->getGlobalName();

        //////////// subBlock build
        ///
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            preRet += subBlockSimEngine->createOp();
        }

        //////////// basic node recruitment
        for (Node* sysNode: _flowBlockBase->getSysNodes()){
            assert(sysNode != nullptr);
            CtrlFlowRegBase* stateReg = sysNode->getStateRegisterIfThere();

            if (stateReg != nullptr){
                std::string regName = stateReg->getSimEngine()->getVarName();
                preRet += "         ";
                preRet += getVarName() + ".getVal() += " + regName + ".getVal();\n";
            }

        }
        ///////////// sub block recruitment
        for (FlowBlockBase* fb: _flowBlockBase->getSubBlocks()){
            FlowBaseSimEngine* subBlockSimEngine = fb->getSimEngine();
            preRet += "         ";
            preRet += getVarName() + ".getVal() += "
                   + subBlockSimEngine->getVarNameCurStatus() + ".getVal();\n";
        }
        preRet += "     ";
        preRet += getVarName()          + ".getVal() += " +
                  getVarNameCurStatus() + ".getLogicValue()\n";

        preRet += "     }\n";

        for (FlowBlockBase* fb: _flowBlockBase->getConBlocks()){
            FlowBaseSimEngine* conBlockSimEngine = fb->getSimEngine();
            preRet += conBlockSimEngine->createOp();
        }

        return preRet;
    }










}
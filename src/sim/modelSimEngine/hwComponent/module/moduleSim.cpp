//
// Created by tanawin on 31/5/2024.
//

#include "moduleSim.h"

#include "util/type/typeConv.h"
#include "model/hwComponent/module/module.h"


namespace kathryn{
    ModuleSimEngine::ModuleSimEngine(Module* module):
        _module(module){
    }

    void ModuleSimEngine::proxyBuildInit(){
        std::vector<ModelProxyBuild*> result;

        /////////// recruit all variable
        recruitStateFullEle(result);
        recruitStateLessEle(result);
        recruitMemBlk(result);
        recruitMemElh(result, false);
        recruitMemElh(result, true);
        ///// init
        for (ModelProxyBuild* mbp : result){
            ///std::cout << "--------" << std::endl;
            ///std::cout << mbp->getVarName() << std::endl;
            mbp->proxyBuildInit();
        }
        //////////
        for (Module* md : _module->getUserSubModules()){
            assert(md != nullptr);
            md->getSimEngine()->proxyBuildInit();
        }
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForCreateVar(){
        std::vector<ModelProxyBuild*> result;
        recruitStateFullEle(result);
        recruitStateLessEle(result);
        /** mem block and its subsidaries*/
        recruitMemBlk(result);
        recruitMemElh(result, true);
        recruitMemElh(result, false);
        recruitFromSubModule(result, &ModuleSimEngine::recruitForCreateVar);

        return result;
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForRegisVar(){
        std::vector<ModelProxyBuild*> result;
        /////// recruit sp register first
        recruitStateFullEle(result);
        recruitFromVector(result, _module->getUserWires());
        /** mem block and its subsidaries*/
        recruitMemBlk(result);
        //// skip mem ele holder recruitFromMemElh(result, true);
        //// skip mem ele holder recruitFromMemElh(result, false);
        recruitFromSubModule(result, &ModuleSimEngine::recruitForRegisVar);
        return result;
    }


    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForMainOpVolatile(){
        ///// wire and mem block agent
        std::vector<ModelProxyBuild*> result;
        recruitStateLessEle(result);
        recruitMemElh(result, true);
        recruitFromSubModule(result, &ModuleSimEngine::recruitForMainOpVolatile);
        return result;
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForMainOpNonVolatile(){
        return recruitForFinalizeOp();
    }


    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForFinalizeOp(){
        std::vector<ModelProxyBuild*> result;
        recruitStateFullEle(result);
        recruitMemElh(result, false); /// mem block not include here due to ele handle it by themselve
        recruitFromSubModule(result, &ModuleSimEngine::recruitForFinalizeOp);
        return result;
    }

    std::vector<LogicSimEngine*> ModuleSimEngine::recruitForVcdVar(){
        std::vector<LogicSimEngine*> result;
        for (int spIdx = 0; spIdx < SP_CNT_REG; spIdx++){
            recruitFromVector(result,
            _module->getSpRegs(static_cast<SP_REG_TYPE>(spIdx)));
        }
        recruitFromVector(result, _module->getUserRegs());
        recruitFromVector(result, _module->getUserWires());
        recruitFromSubModule(result, &ModuleSimEngine::recruitForVcdVar);
        return result;
    }

    std::vector<FlowBaseSimEngine*> ModuleSimEngine::recruitPerf(){
        std::vector<FlowBaseSimEngine*> result;
        ///////// recurte flow block in module
        recruitFromVector(result, _module->getFlowBlocks());
        recruitFromSubModule(result, &ModuleSimEngine::recruitPerf);
        return result;
    }


    /**
     *
     *
     * MAIN RECRUITER
     *
     */

    void ModuleSimEngine::recruitStateFullEle
    (std::vector<ModelProxyBuild*>& result){
        recruitFromSpReg(result);
        recruitFromVector(result, _module->getUserRegs());
    }

    void ModuleSimEngine::recruitStateLessEle
    (std::vector<ModelProxyBuild*>& result){
        recruitFromVector(result, _module->getUserWires());
        recruitFromVector(result, _module->getUserExpressions());
        recruitFromVector(result, _module->getUserVals());
        recruitFromVector(result, _module->getUserPmVals());
        recruitFromVector(result, _module->getUserNests());
    }

    void ModuleSimEngine::recruitMemBlk(std::vector<ModelProxyBuild*>& result){
        recruitFromVector(result, _module->getUserMemBlks());
    }

    void ModuleSimEngine::recruitMemElh(std::vector<ModelProxyBuild*>& result, bool isReadMode){
        for (MemBlock* memBlock : _module->getUserMemBlks()){
            for (MemBlockEleHolder* holderPtr : memBlock->getMemBlockAgents()){
                assert(holderPtr != nullptr);
                if (holderPtr->isReadMode() == isReadMode){
                    result.push_back(holderPtr->getSimEngine());
                }
            }
        }
    }


    void ModuleSimEngine::recruitFromSpReg
    (std::vector<ModelProxyBuild*>& result){
        for (int spIdx = 0; spIdx < SP_CNT_REG; spIdx++){
            recruitFromVector(result,
            _module->getSpRegs(static_cast<SP_REG_TYPE>(spIdx)));
        }
    }


    /**
     *
     *
     * retrieve zone
     *
     ***/


    void ModuleSimEngine::retrieveInit(ProxySimEventBase* simEventBase){
        /***
         *
         * must same as regist function
         *
         */

        retrieveInitFromVector(simEventBase, _module->getSpRegs(SP_STATE_REG));
        retrieveInitFromVector(simEventBase, _module->getSpRegs(SP_SYNC_REG));
        retrieveInitFromVector(simEventBase, _module->getSpRegs(SP_COND_WAIT_REG));
        retrieveInitFromVector(simEventBase, _module->getSpRegs(SP_CYCLE_WAIT_REG));
        retrieveInitFromVector(simEventBase, _module->getUserRegs());
        /////////// wire
        retrieveInitFromVector(simEventBase, _module->getUserWires());
        ////////// memory
        retrieveInitFromVector(simEventBase, _module->getUserMemBlks());

        //////// for flowblock
        retrieveInitFromVector(simEventBase, _module->getFlowBlocks());

        ////////// subModule
        for (Module* subModule : _module->getUserSubModules()){
            subModule->getSimEngine()->retrieveInit(simEventBase);
        }
    }


    template <typename T>
    void ModuleSimEngine::recruitFromSubModule(
        std::vector<T*>& result,
        std::vector<T*> (ModuleSimEngine::*func)()){
        for (Module* subModule : _module->getUserSubModules()){
            ModuleSimEngine* mse = subModule->getSimEngine();
            std::vector<T*> subResult = (mse->*func)();
            appendVector(result, subResult);
        }
    }

    template <typename S, typename T>
    void ModuleSimEngine::recruitFromVector(
        std::vector<S*>& result,
        std::vector<T>& eleVec){
        for (auto elePtr : eleVec){
            assert(elePtr != nullptr);
            result.push_back((S*)elePtr->getSimEngine());
        }
    }

    template <typename T>
    void ModuleSimEngine::retrieveInitFromVector(
        ProxySimEventBase* simEventBase,
        std::vector<T*>& eleVec){
        for (auto elePtr : eleVec){
            assert(elePtr != nullptr);
            elePtr->getSimEngine()->proxyRetInit(simEventBase);
        }
    }
}

//
// Created by tanawin on 31/5/2024.
//

#include "moduleSimEngine.h"

#include "util/type/typeConv.h"
#include "model/hwComponent/module/module.h"


namespace kathryn{
    ModuleSimEngine::ModuleSimEngine(Module* module):
        _module(module){
    }

    void ModuleSimEngine::proxyBuildInit(){
        std::vector<ModelProxyBuild*> result;

        /////////// recruit all variable
        recruitFromRegable(result);
        recruitFromWireable(result);
        recruitFromMemBlk(result);
        recruitFromMemElh(result, false);
        recruitFromMemElh(result, true);
        ///// init
        for (ModelProxyBuild* mbp : result){
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
        /////// recruit sp register first
        recruitFromRegable(result);
        recruitFromWireable(result);
        /** mem block and its subsidaries*/
        recruitFromMemBlk(result);
        recruitFromMemElh(result, true);
        recruitFromMemElh(result, false);
        recruitFromSubModule(result, &ModuleSimEngine::recruitForCreateVar);

        return result;
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForRegisVar(){
        std::vector<ModelProxyBuild*> result;
        /////// recruit sp register first
        recruitFromRegable(result);
        recruitFromVector(result, _module->getUserWires());
        /** mem block and its subsidaries*/
        recruitFromMemBlk(result);
        //// skip mem ele holder recruitFromMemElh(result, true);
        //// skip mem ele holder recruitFromMemElh(result, false);
        recruitFromSubModule(result, &ModuleSimEngine::recruitForRegisVar);
        return result;
    }


    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForMainOpVolatile(){
        ///// wire and mem block agent
        std::vector<ModelProxyBuild*> result;
        recruitFromWireable(result);
        recruitFromMemElh(result, true);
        recruitFromSubModule(result, &ModuleSimEngine::recruitForMainOpVolatile);
        return result;
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForMainOpNonVolatile(){
        return recruitForFinalizeOp();
    }


    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForFinalizeOp(){
        std::vector<ModelProxyBuild*> result;
        recruitFromRegable(result);
        recruitFromMemElh(result, false); /// mem block not include here due to ele handle it by themselve
        recruitFromSubModule(result, &ModuleSimEngine::recruitForFinalizeOp);
        return result;
    }

    std::vector<LogicSimEngine*> ModuleSimEngine::recruitForVcdVar(){
        std::vector<LogicSimEngine*> result;
        recruitFromVector(result, _module->getSpRegs(SP_STATE_REG));
        recruitFromVector(result, _module->getSpRegs(SP_SYNC_REG));
        recruitFromVector(result, _module->getSpRegs(SP_COND_WAIT_REG));
        recruitFromVector(result, _module->getSpRegs(SP_CYCLE_WAIT_REG));
        recruitFromVector(result, _module->getUserRegs());
        recruitFromVector(result, _module->getUserWires());
        recruitFromSubModule(result, &ModuleSimEngine::recruitForVcdVar);
        return result;
    }

    // std::vector<LogicSimEngine*> ModuleSimEngine::recruitAllLogicSimEngine(){
    //
    //     std::vector<LogicSimEngine*> result;
    //     /////// recruit sp register first
    //     recruitFromVector(result, _module->getSpRegs(SP_STATE_REG));
    //     recruitFromVector(result, _module->getSpRegs(SP_SYNC_REG));
    //     recruitFromVector(result, _module->getSpRegs(SP_COND_WAIT_REG));
    //     recruitFromVector(result, _module->getSpRegs(SP_CYCLE_WAIT_REG));
    //     recruitFromVector(result, _module->getUserRegs());
    //     /////////// wire
    //     recruitFromVector(result, _module->getUserWires());
    //     recruitFromVector(result, _module->getUserExpressions());
    //     recruitFromVector(result, _module->getUserVals());
    //     recruitFromVector(result, _module->getUserNests());
    //     ////////// memory
    //     for (MemBlock* memBlock: _module->getUserMemBlks()){
    //         recruitFromVector(result, memBlock->getMemBlockAgents());
    //     }
    //     recruitFromSubModule(result, &ModuleSimEngine::recruitAllLogicSimEngine);
    //     return result;
    //
    // }

    std::vector<FlowBaseSimEngine*> ModuleSimEngine::recruitPerf(){
        std::vector<FlowBaseSimEngine*> result;
        ///////// recurte flow block in module
        recruitFromVector(result, _module->getFlowBlocks());
        recruitFromSubModule(result, &ModuleSimEngine::recruitPerf);
        return result;
    }

    void ModuleSimEngine::recruitFromRegable
    (std::vector<ModelProxyBuild*>& result){
        recruitFromSpReg(result);
        recruitFromVector(result, _module->getUserRegs());
    }

    void ModuleSimEngine::recruitFromSpReg
    (std::vector<ModelProxyBuild*>& result){
        for (int spIdx = 0; spIdx < SP_CNT_REG; spIdx++){
            recruitFromVector(result,
                              _module->getSpRegs(static_cast<SP_REG_TYPE>(spIdx)));
        }
    }

    void ModuleSimEngine::recruitFromWireable
    (std::vector<ModelProxyBuild*>& result){
        recruitFromVector(result, _module->getUserWires());
        recruitFromVector(result, _module->getUserExpressions());
        recruitFromVector(result, _module->getUserVals());
        recruitFromVector(result, _module->getUserNests());
    }

    void ModuleSimEngine::recruitFromMemBlk(std::vector<ModelProxyBuild*>& result){
        recruitFromVector(result, _module->getUserMemBlks());
    }

    void ModuleSimEngine::recruitFromMemElh(std::vector<ModelProxyBuild*>& result, bool isReadMode){
        for (MemBlock* memBlock : _module->getUserMemBlks()){
            for (MemBlockEleHolder* holderPtr : memBlock->getMemBlockAgents()){
                assert(holderPtr != nullptr);
                if (holderPtr->isReadMode() == isReadMode){
                    result.push_back(holderPtr->getSimEngine());
                }
            }
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

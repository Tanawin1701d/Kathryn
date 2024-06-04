//
// Created by tanawin on 31/5/2024.
//

#include "moduleSimEngine.h"

#include <util/type/typeConv.h>


#include "model/hwComponent/module/module.h"


namespace kathryn{

    ModuleSimEngine::ModuleSimEngine(Module* module):
    _module(module){}


    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForCreateVar(){

        std::vector<ModelProxyBuild*> result;
        /////// recruit sp register first
        recruitFromRegable (result);
        recruitFromWireable(result);
        /** mem block and its subsidaries*/
        recruitFromVector(result, _module->getUserMemBlks());
        for (MemBlock* memBlock: _module->getUserMemBlks()){
            recruitFromVector(result, memBlock->getMemBlockAgents());
        }


        for (Module* subModule: _module->getUserSubModules()){
            std::vector<ModelProxyBuild*> subResult = subModule->getSimEngine()
            ->recruitForCreateVar();
            appendVector(result, subResult);
        }
        return result;

    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForVolatileEle(){
        ///// wire and mem block agent
        std::vector<ModelProxyBuild*> result;
        recruitFromWireable(result);
        for (MemBlock* memBlock: _module->getUserMemBlks()){
            for (MemBlockEleHolder* holderPtr: memBlock->getMemBlockAgents()){
                assert(holderPtr != nullptr);
                if (holderPtr->isReadMode()){
                    result.push_back(holderPtr->getSimEngine());
                }
            }
        }
        for (Module* subModule: _module->getUserSubModules()){
            std::vector<ModelProxyBuild*> subResult = subModule->getSimEngine()
            ->recruitForVolatileEle();
            appendVector(result, subResult);
        }
        return result;
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitForNonVolatileEle(){

        std::vector<ModelProxyBuild*> result;
        recruitFromRegable(result);
        for (MemBlock* memBlock: _module->getUserMemBlks()){
            for (MemBlockEleHolder* holderPtr: memBlock->getMemBlockAgents()){
                assert(holderPtr != nullptr);
                if (holderPtr->isWriteMode()){
                    result.push_back(holderPtr->getSimEngine());
                }
            }
        }
        for (Module* subModule: _module->getUserSubModules()){
            std::vector<ModelProxyBuild*> subResult = subModule->getSimEngine()
            ->recruitForNonVolatileEle();
            appendVector(result, subResult);
        }
        return result;
    }

    std::vector<LogicSimEngine*> ModuleSimEngine::recruitAllLogicSimEngine(){

        std::vector<LogicSimEngine*> result;
        /////// recruit sp register first
        recruitFromVector(result, _module->getSpRegs(SP_STATE_REG));
        recruitFromVector(result, _module->getSpRegs(SP_SYNC_REG));
        recruitFromVector(result, _module->getSpRegs(SP_COND_WAIT_REG));
        recruitFromVector(result, _module->getSpRegs(SP_CYCLE_WAIT_REG));
        recruitFromVector(result, _module->getUserRegs());
        /////////// wire
        recruitFromVector(result, _module->getUserWires());
        recruitFromVector(result, _module->getUserExpressions());
        recruitFromVector(result, _module->getUserVals());
        recruitFromVector(result, _module->getUserNests());

        ////////// memory
        for (MemBlock* memBlock: _module->getUserMemBlks()){
            recruitFromVector(result, memBlock->getMemBlockAgents());
        }
        for (Module* subModule: _module->getUserSubModules()){
            std::vector<LogicSimEngine*> subResult = subModule->getSimEngine()
            ->recruitAllLogicSimEngine();
            appendVector(result, subResult);
        }
        return result;

    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruitPerf(){
        std::vector<ModelProxyBuild*> result;
        ///////// recurte flow block in module
        recruitFromVector(result, _module->getFlowBlocks());
        for (Module* subModule: _module->getUserSubModules()){
            std::vector<ModelProxyBuild*> subResult = subModule->getSimEngine()
            ->recruitPerf();
            appendVector(result, subResult);
        }
        return result;
    }

    void ModuleSimEngine::recruitFromRegable
    (std::vector<ModelProxyBuild*>& result){
        recruitFromVector(result, _module->getSpRegs(SP_STATE_REG));
        recruitFromVector(result, _module->getSpRegs(SP_SYNC_REG));
        recruitFromVector(result, _module->getSpRegs(SP_COND_WAIT_REG));
        recruitFromVector(result, _module->getSpRegs(SP_CYCLE_WAIT_REG));
        recruitFromVector(result, _module->getUserRegs());
    }

    void ModuleSimEngine::recruitFromWireable
    (std::vector<ModelProxyBuild*>& result){
        recruitFromVector(result, _module->getUserWires());
        recruitFromVector(result, _module->getUserExpressions());
        recruitFromVector(result, _module->getUserVals());
        recruitFromVector(result, _module->getUserNests());
    }


    void ModuleSimEngine::retrieveInit(){
        retrieveInitFromVector(_module->getSpRegs(SP_STATE_REG));
        retrieveInitFromVector(_module->getSpRegs(SP_SYNC_REG));
        retrieveInitFromVector(_module->getSpRegs(SP_COND_WAIT_REG));
        retrieveInitFromVector(_module->getSpRegs(SP_CYCLE_WAIT_REG));
        retrieveInitFromVector(_module->getUserRegs());
        /////////// wire
        retrieveInitFromVector(_module->getUserWires());
        retrieveInitFromVector(_module->getUserExpressions());
        retrieveInitFromVector(_module->getUserVals());
        retrieveInitFromVector(_module->getUserNests());

        ////////// memory
        retrieveInitFromVector(_module->getUserMemBlks());
        for (MemBlock* memBlock: _module->getUserMemBlks()){
            retrieveInitFromVector(memBlock->getMemBlockAgents());
        }

        //////// for flowblock
        retrieveInitFromVector(_module->getFlowBlocks());

        ////////// subModule
        for (Module* subModule: _module->getUserSubModules()){
            subModule->getSimEngine()->retrieveInit();
        }
    }

    template<typename S, typename T>
    void ModuleSimEngine::recruitFromVector(
        std::vector<S*>& result,
        std::vector<T>& eleVec){
        for (auto elePtr: eleVec){
            assert(elePtr != nullptr);
            result.push_back((S*)elePtr->getSimEngine());
        }
    }

    template <typename T>
    void ModuleSimEngine::retrieveInitFromVector(std::vector<T*>& eleVec){
        for (auto elePtr: eleVec){
            assert(elePtr != nullptr);
            elePtr->getSimEngine()->proxyRetInit();
        }
    }






}
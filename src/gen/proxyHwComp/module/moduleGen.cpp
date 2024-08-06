//
// Created by tanawin on 20/6/2024.
//

#include "moduleGen.h"

#include "model/hwComponent/module/module.h"
#include "model/hwComponent/abstract/globPool.h"
#include "gen/proxyHwComp/abstract/logicGenBase.h"

namespace kathryn{


    ModuleGen::ModuleGen(Module* master):
    _master(master){
        assert(_master != nullptr);
    }

    void ModuleGen::startInitEle(){

        if (_master->getParent() == nullptr){
            depthFromGlobalModule = 0;
        }else{
            depthFromGlobalModule = _master->getParent()
                                    ->getModuleGen()->getDept() + 1;
        }

        //////// init the sub module elements first
        for (Module* subModule: _master->getUserSubModules()){
            subModule->createModuleGen(); //// create submodule gen first
            subModule->getModuleGen()->startInitEle(); //// init sub module element
            _subModulePool.push_back(subModule->getModuleGen()); //// add to this module pool
        }

        //////// init all logic element wo test
        /////////////////// logic element should not be recursively called, it should be atomic
        for (int spIdx = 0; spIdx < SP_CNT_REG; spIdx++){
            createAndRecruitLogicGenBase(
                _regPool,
                _master->getSpRegs((SP_REG_TYPE)spIdx));
        }
        createAndRecruitLogicGenBase(_regPool, _master->getUserRegs());
        ////// for wire we must seperate marker for input and output of module mark
        for (Wire* wire: _master->getUserWires()){
            wire->createLogicGen();
            switch (wire->getMarker()){
                case WMT_INPUT_MD:{
                    _wirePoolWithInputMarker.push_back(wire->getLogicGen());
                    break;
                }
                case WMT_OUTPUT_MD:{
                    _wirePoolWithOutputMarker.push_back(wire->getLogicGen());
                    break;
                }
                default:{
                    _wirePool.push_back(wire->getLogicGen());
                    break;
                }
            }
        }
        createAndRecruitLogicGenBase(_exprPool,_master->getUserExpressions());
        createAndRecruitLogicGenBase(_nestPool,_master->getUserNests());
        createAndRecruitLogicGenBase(_valPool, _master->getUserVals());
        createAndRecruitLogicGenBase(_memBlockPool, _master->getUserMemBlks());
        for (MemBlock* memBlock: _master->getUserMemBlks()){
            createAndRecruitLogicGenBase(_memBlockElePool, memBlock->getMemBlockAgents());
        }

    }
}

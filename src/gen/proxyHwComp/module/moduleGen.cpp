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

    template<typename T>
    void ModuleGen::createLogicGenBase(std::vector<T*>& srcs){
        for(T* src: srcs){
            src->createLogicGen();
        }
    }
    template<typename T>
    void ModuleGen::createAndRecruitLogicGenBase(
        LogicGenBaseVec& des,
        std::vector<T*>& srcs){
        createLogicGenBase(srcs);
        recruitLogicGenBase(des, srcs);
    }

    void ModuleGen::initializeIoWire(std::unordered_map<Operable*, int>& ioMap,
                                     std::vector<WireIo*>& ioVec,
                                     std::vector<WireIo*>& srcVec){
        ioVec = srcVec;
        for(int idx = 0; idx < ioVec.size(); idx++){
            ioMap.insert({ioVec[idx], idx});
        }
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
        createAndRecruitLogicGenBase(_wirePool,_master->getUserWires());
        createAndRecruitLogicGenBase(_exprPool,_master->getUserExpressions());
        createAndRecruitLogicGenBase(_nestPool,_master->getUserNests());
        createAndRecruitLogicGenBase(_valPool, _master->getUserVals());
        createAndRecruitLogicGenBase(_memBlockPool, _master->getUserMemBlks());

        for (MemBlock* memBlock: _master->getUserMemBlks()){
            createAndRecruitLogicGenBase(_memBlockElePool, memBlock->getMemBlockAgents());
        }

        /////// io wire initialization
        initializeIoWire(_ioWireMaps[WIRE_IO_USER_INPUT],
                         _ioWires[WIRE_IO_USER_INPUT],
                         _master->getUserIoInputWires());
        initializeIoWire(_ioWireMaps[WIRE_IO_USER_OUPUT],
                        _ioWires[WIRE_IO_USER_OUPUT],
                        _master->getUserIoOutputWires());

    }
}

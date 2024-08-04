//
// Created by tanawin on 30/6/2024.
//

#include "moduleGen.h"
#include "gen/controller/genStructure.h"
#include "model/hwComponent/module/module.h"
namespace kathryn{


    void ModuleGen::genCerfAll(int idx){
        genCerfToThisModule(idx);
        genCerfToEachElement();
        //////// gen cerf for each submodule
        for (int subIdx = 0; subIdx < _subModulePool.size(); subIdx++){
            ModuleGen* mdGen = _subModulePool[subIdx];
            assert(mdGen != nullptr);
            mdGen->genCerfAll(subIdx);
        }
    }

    void ModuleGen::startPutToGenSystem(GenStructure* genStructure){

        assert(genStructure != nullptr);
        //////////// DO IT FOR SUB MODULE FIRST

        for (ModuleGen* subMdGen: _subModulePool){
            assert(subMdGen != nullptr);
            subMdGen->startPutToGenSystem(genStructure);
        }

        genStructure->addNewModule(this);

    }

    bool ModuleGen::startCmpModule(ModuleGen* rhsMdg, GenStructure* genStructure){
        assert(rhsMdg != nullptr);
        assert(genStructure != nullptr);

        if ( (_cerf.varMeta.isUser  != rhsMdg->_cerf.varMeta.isUser) ||
             (_cerf.varMeta.varType != rhsMdg->_cerf.varMeta.varType)
        ){ return false;}

        //std::cout << "start deep cmp @ " << _cerf.varMeta.varName << "  with " << rhsMdg->_cerf.varMeta.varName << std::endl;

        //////// compare standard
        bool prelimResult = true;
        prelimResult &= _regPool        .compare(rhsMdg->_regPool);
        prelimResult &= _wirePool       .compare(rhsMdg->_wirePool);
        prelimResult &= _exprPool       .compare(rhsMdg->_exprPool);
        prelimResult &= _nestPool       .compare(rhsMdg->_nestPool);
        prelimResult &= _valPool        .compare(rhsMdg->_valPool);
        prelimResult &= _memBlockPool   .compare(rhsMdg->_memBlockPool);
        prelimResult &= _memBlockElePool.compare(rhsMdg->_memBlockElePool);

        /////// compare io wire //////// compare only output
        WIRE_IO_TYPE ioWireOutList []= {WIRE_IO_AUTO_OUTPUT, WIRE_IO_INTER,
                                        WIRE_IO_OUTPUT_GLOB, WIRE_IO_USER_OUPUT};
        WIRE_IO_TYPE ioWireInList  [] = {WIRE_IO_AUTO_INPUT, WIRE_IO_INPUT_GLOB,
                                         WIRE_IO_USER_INPUT};

        for (int ioWireType: ioWireOutList){
            prelimResult &= _ioWirePools[ioWireType].compare(rhsMdg->_ioWirePools[ioWireType]);
        }
        for (int ioWireType: ioWireInList){
            prelimResult &= _ioWirePools[ioWireType].compareCefOnly(rhsMdg->_ioWirePools[ioWireType]);
        }

        ///std::cout << "prelim res " << prelimResult << std::endl;
        if(!prelimResult) {return false;}

        ////// compare sub module input
        if (_subModulePool.size() != rhsMdg->_subModulePool.size()){return false;}

        bool subResult = true;

        for (int idx = 0; idx < _subModulePool.size(); idx++){
            ModuleGen* lhsSubModuleGen  = _subModulePool[idx];
            ModuleGen* rhsSubModuleGen  = rhsMdg->_subModulePool[idx];
            /////// compare the context of sub module
            subResult &= genStructure->isTheSameModule(lhsSubModuleGen, rhsSubModuleGen);
            /////// compare input
            for (int ioWireType: ioWireInList){
                subResult &= lhsSubModuleGen->_ioWirePools[ioWireType].compare(rhsSubModuleGen->_ioWirePools[ioWireType]);
            }
            for (int ioWireType: ioWireInList){
                subResult &= lhsSubModuleGen->_ioWirePools[ioWireType].compareCefOnly(rhsSubModuleGen->_ioWirePools[ioWireType]);
            }
            if (!subResult){return false;}
        }

        return true;

    }


    ////////////////////////////////////////////////////////////////
    ///////////////////// every element cerf
    void ModuleGen::genCerfToEachElement(){

        _regPool        .genCerf(GEN_REG_GRP, 0);
        _wirePool       .genCerf(GEN_WIRE_GRP, 1);
        _exprPool       .genCerf(GEN_EXPRE_GRP, 2);
        _nestPool       .genCerf(GEN_NEST_GRP, 3);
        _valPool        .genCerf(GEN_VAL_GRP, 4);
        _memBlockPool   .genCerf(GEN_MEMBLK_GRP, 5);
        _memBlockElePool.genCerf(GEN_MEMBLK_ELE_GRP, 6);
        //////// io wire

        _ioWirePools[WIRE_IO_AUTO_INPUT] .genCerf( GEN_WIRE_IO_AUTO_INPUT, 7);
        _ioWirePools[WIRE_IO_AUTO_OUTPUT].genCerf( GEN_WIRE_IO_AUTO_OUTPUT,8);
        _ioWirePools[WIRE_IO_INTER]      .genCerf( GEN_WIRE_IO_INTER,      9);
        _ioWirePools[WIRE_IO_INPUT_GLOB] .genCerf( GEN_WIRE_IO_INPUT_GLOB, 10);
        _ioWirePools[WIRE_IO_OUTPUT_GLOB].genCerf( GEN_WIRE_IO_OUTPUT_GLOB,11);
        _ioWirePools[WIRE_IO_USER_INPUT] .genCerf( GEN_WIRE_IO_USER_INPUT, 12);
        _ioWirePools[WIRE_IO_USER_OUPUT] .genCerf( GEN_WIRE_IO_USER_OUPUT, 13);
    }

    void ModuleGen::genCerfToThisModule(int idx){
        ////// local cerf
        _cerf.varMeta    = _master->getVarMeta();
        _cerf.idx        = idx;
        ////// global cerf
        _globCerf.varMeta = _master->getVarMeta();
    }

    bool ModuleGen::cmpCerfEqLocally(const ModuleGen& rhs) const{
        return _cerf.cmpAsSubModule(rhs.getCerf());
    }

}
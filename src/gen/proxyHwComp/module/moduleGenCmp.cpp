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
        prelimResult &= _globalOutputPool  .compare       (rhsMdg->_globalOutputPool);
        prelimResult &= _autoOutputWirePool.compare       (rhsMdg->_autoOutputWirePool);
        prelimResult &= _interWirePool     .compare       (rhsMdg->_interWirePool);
        prelimResult &= _globalInputPool   .compareCefOnly(rhsMdg->_globalInputPool);
        prelimResult &= _autoInputWirePool .compareCefOnly(rhsMdg->_autoInputWirePool);

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
            subResult &= lhsSubModuleGen->_globalInputPool  .compare(rhsSubModuleGen->_globalInputPool);
            subResult &= lhsSubModuleGen->_autoInputWirePool.compare(rhsSubModuleGen->_autoInputWirePool);
            /////// compare output
            subResult &= lhsSubModuleGen->_globalOutputPool.compareCefOnly(rhsSubModuleGen->_globalOutputPool);
            subResult &= lhsSubModuleGen->_autoOutputWirePool.compareCefOnly(rhsSubModuleGen->_autoOutputWirePool);

            if (!subResult){return false;}
        }

        return true;

    }


    ////////////////////////////////////////////////////////////////
    ///////////////////// every element cerf
    void ModuleGen::genCerfToEachElement(){

        LogicGenBaseVec _interWireVec;
        LogicGenBaseVec _autoInputGenVec;
        LogicGenBaseVec _autoOutputGenVec;
        LogicGenBaseVec _globInputGenVec;
        LogicGenBaseVec _globOutputGenVec;
        recruitLogicGenBase(_interWireVec, _interWires);
        recruitLogicGenBase(_autoInputGenVec, _autoInputWires);
        recruitLogicGenBase(_autoOutputGenVec, _autoOutputWires);
        recruitLogicGenBase(_globInputGenVec, _globalInputs);
        recruitLogicGenBase(_globOutputGenVec, _globalOutputs);

        _regPool        .genCerf(GEN_REG_GRP, 0);
        _wirePool       .genCerf(GEN_WIRE_GRP, 1);
        _exprPool       .genCerf(GEN_EXPRE_GRP, 2);
        _nestPool       .genCerf(GEN_NEST_GRP, 3);
        _valPool        .genCerf(GEN_VAL_GRP, 4);
        _memBlockPool   .genCerf(GEN_MEMBLK_GRP, 5);
        _memBlockElePool.genCerf(GEN_MEMBLK_ELE_GRP, 6);
        //////// io wire
        _interWirePool     .genCerf(GEN_INTER_WIRE_GRP, 7);
        _autoInputWirePool .genCerf(GEN_AUTO_INPUT_WIRE_GRP, 8);
        _autoOutputWirePool.genCerf(GEN_AUTO_OUTPUT_WIRE_GRP, 9);
        _globalInputPool   .genCerf(GEN_AUTO_INPUT_WIRE_GRP, 10);
        _globalOutputPool  .genCerf(GEN_AUTO_OUTPUT_WIRE_GRP, 11);
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
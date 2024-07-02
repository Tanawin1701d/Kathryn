//
// Created by tanawin on 30/6/2024.
//

#include "moduleGen.h"
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

    bool ModuleGen::startCmpModule(ModuleGen* rhsMdg){
        assert(rhsMdg != nullptr);


        if ( (_cerf.varMeta.isUser  != rhsMdg->_cerf.varMeta.isUser) ||
             (_cerf.varMeta.varType != rhsMdg->_cerf.varMeta.varType)
        ){ return false;}


        /////// to do check cerf

        LogicGenBaseVec lhsGlobalInputs    = recruitLogicGenBase( _globalInputs);
        LogicGenBaseVec lhsGlobalOutputs   = recruitLogicGenBase( _globalOutputs);
        LogicGenBaseVec lhsAutoInputWires  = recruitLogicGenBase( _autoInputWires);
        LogicGenBaseVec lhsAutoOutputWires = recruitLogicGenBase( _autoOutputWires);
        LogicGenBaseVec lhsInterWires      = recruitLogicGenBase( _interWires);

        LogicGenBaseVec rhsGlobalInputs    = recruitLogicGenBase( rhsMdg->_globalInputs);
        LogicGenBaseVec rhsGlobalOutputs   = recruitLogicGenBase( rhsMdg->_globalOutputs);
        LogicGenBaseVec rhsAutoInputWires  = recruitLogicGenBase( rhsMdg->_autoInputWires);
        LogicGenBaseVec rhsAutoOutputWires = recruitLogicGenBase( rhsMdg->_autoOutputWires);
        LogicGenBaseVec rhsInterWires      = recruitLogicGenBase( rhsMdg->_interWires);

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
        prelimResult &= lhsGlobalOutputs  .compare       (rhsGlobalOutputs);
        prelimResult &= lhsAutoOutputWires.compare       (rhsAutoOutputWires);
        prelimResult &= lhsInterWires     .compare       (rhsInterWires);
        prelimResult &= lhsGlobalInputs   .compareCefOnly(rhsGlobalInputs);
        prelimResult &= lhsAutoInputWires .compareCefOnly(rhsAutoInputWires);

        if(!prelimResult) {return false;}

        ////// compare sub module input
        if (_subModulePool.size() != rhsMdg->_subModulePool.size()){return false;}

        bool subResult = true;

        for (int idx = 0; idx < _subModulePool.size(); idx++){
            ModuleGen* lhsSubModuleGen  = _subModulePool[idx];
            ModuleGen* rhsSubModuleGen  = rhsMdg->_subModulePool[idx];

            /////////////// compare input
            LogicGenBaseVec lhsSubInputAuto = lhsSubModuleGen->recruitLogicGenBase(lhsSubModuleGen->_autoInputWires);
            LogicGenBaseVec rhsSubInputAuto = rhsSubModuleGen->recruitLogicGenBase(rhsSubModuleGen->_autoInputWires);
            LogicGenBaseVec lhsSubInputGlob = lhsSubModuleGen->recruitLogicGenBase(lhsSubModuleGen->_globalInputs);
            LogicGenBaseVec rhsSubInputGlob = rhsSubModuleGen->recruitLogicGenBase(rhsSubModuleGen->_globalInputs);

            subResult &= lhsSubInputAuto.compare(rhsSubInputAuto);
            subResult &= lhsSubInputGlob.compare(rhsSubInputGlob);

            LogicGenBaseVec lhsSubOutputAuto = lhsSubModuleGen->recruitLogicGenBase(lhsSubModuleGen->_autoOutputWires);
            LogicGenBaseVec rhsSubOutputAuto = rhsSubModuleGen->recruitLogicGenBase(rhsSubModuleGen->_autoOutputWires);
            LogicGenBaseVec lhsSubOutputGlob = lhsSubModuleGen->recruitLogicGenBase(lhsSubModuleGen->_globalOutputs);
            LogicGenBaseVec rhsSubOutputGlob = rhsSubModuleGen->recruitLogicGenBase(rhsSubModuleGen->_globalOutputs);

            subResult &= lhsSubOutputAuto.compareCefOnly(rhsSubOutputAuto);
            subResult &= lhsSubOutputGlob.compareCefOnly(rhsSubOutputGlob);

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
        _interWireVec.genCerf(GEN_INTER_WIRE_GRP, 7);
        _autoInputGenVec.genCerf(GEN_AUTO_INPUT_WIRE_GRP, 8);
        _autoOutputGenVec.genCerf(GEN_AUTO_OUTPUT_WIRE_GRP, 9);
        _globInputGenVec.genCerf(GEN_AUTO_INPUT_WIRE_GRP, 8);
        _globOutputGenVec.genCerf(GEN_AUTO_OUTPUT_WIRE_GRP, 9);
    }

    void ModuleGen::genCerfToThisModule(int idx){
        _cerf.varMeta    = _master->getVarMeta();
        _cerf.idx        = idx;
    }

    bool ModuleGen::cmpCerfEqLocally(const ModuleGen& rhs) const{
        return _cerf.cmpAsSubModule(rhs.getCerf());
    }

}
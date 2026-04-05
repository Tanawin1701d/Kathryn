//
// Created by tanawin on 1/7/2024.
//

#include "model/hwComponent/wire/wireAuto.h"
#include "model/hwComponent/module/module.h"

namespace kathryn{

    void ModuleGen::startRouteEle(){
        /////////// route io
        if (_master->isTopModule()){
            ////////////////////////// recruit first
            LogicGenBaseVec inputLogicGenBase;
            LogicGenBaseVec outputLogicGenBase;
            recruitLogicGenBase(inputLogicGenBase , _genWires[WIRE_AUTO_GEN_GLOB_INPUT]);
            recruitLogicGenBase(outputLogicGenBase, _genWires[WIRE_AUTO_GEN_GLOB_OUTPUT]);
            /////////////////////////// route dep for global io
            inputLogicGenBase.routeDepAll();
            outputLogicGenBase.routeDepAll();
        }
        /////////// module gen
        for(ModuleGen* mdGen: _subModulePool){
            mdGen->startRouteEle();
        }
        /////////// module gen
        _regPool                 .routeDepAll();
        _wirePool                .routeDepAll();
        _wirePoolWithInputMarker .routeDepAll();
        _wirePoolWithOutputMarker.routeDepAll();
        _exprPool                .routeDepAll();
        _nestPool                .routeDepAll();
        _valPool                 .routeDepAll(); //// pm doesn't require
        _memBlockPool            .routeDepAll();
        _memBlockElePool         .routeDepAll();
    }

    void ModuleGen::finalizeRouteEle() {
        for(ModuleGen* mdGen: _subModulePool){
            mdGen->finalizeRouteEle();
        }
        for(int genWireType = 0; genWireType < WIRE_AUTO_GEN_CNT; genWireType++){
            recruitLogicGenBase(
                _genWirePools[genWireType], _genWires[genWireType]);
        }
    }

    WireAuto* ModuleGen::genAutoWireBase(
        Operable* opr1,      ///////// opr is exact opr
        Operable* realSrc,  ///////// realSrc is exact opr too
        const std::string& wireName,
        WIRE_AUTO_GEN_TYPE wireGenType,
        bool connectTheWire){

        assert(opr1     != nullptr);
        assert(realSrc != nullptr);
        assert(wireGenType < WIRE_AUTO_GEN_CNT);

        std::unorderedMap<Operable*, int>& genMap = _genWireMaps[wireGenType];
        std::vector<WireAuto*>&          genVec = _genWires[wireGenType];

        WireAuto& newAddedWire = makeOprIoWire("addAutoWireBase_uninit",
                                              opr1->getOperableSlice().getSize(),
                                              wireGenType);

            ///////// addd update Event for only connection
            newAddedWire.buildHierarchy(_master);
            newAddedWire.createLogicGen();
            if (connectTheWire){
                newAddedWire.connectTo(opr1, true);
            }

            newAddedWire.setVarName(wireName +
                                    std::toString(genVec.size()) +
                                    "_" +
                                    realSrc->get_logic_gen_ptrBase()->
                                    getIdentPtr()->getVarName()
                                    );
            genVec.pushBack(&newAddedWire);
            genMap.insert({realSrc, genVec.size()-1});
            //////////////////////////////////////////////
            return &newAddedWire;
    }

    bool ModuleGen::isThereAutoGenWire(Operable* realSrc, WIRE_AUTO_GEN_TYPE wireGenType){
        std::unorderedMap<Operable*, int>& genMap = _genWireMaps[wireGenType];
        return genMap.find(realSrc) != genMap.end();
    }

    WireAuto* ModuleGen::getAutoGenWire(Operable* realSrc, WIRE_AUTO_GEN_TYPE wireGenType){
        std::unorderedMap<Operable*, int>& genMap = _genWireMaps[wireGenType];
        std::vector<WireAuto*>&          genVec = _genWires[wireGenType];
        return genVec.at(genMap[realSrc]);
    }

    /////////////////////////////// routing wire

    Operable* ModuleGen::routeSrcOprToThisModule(Operable* realSrc){
        assert(realSrc != nullptr);

        Operable* exactRealSrc = &realSrc->getExactOperable();

        ModuleGen* desModuleGen = this;
        ModuleGen* srcModuleGen = exactRealSrc->get_logic_gen_ptrBase()->getModuleGenPtr();

        ////// if it is same module then return that operable
        if (srcModuleGen == desModuleGen){
            return realSrc;
        }
        //////// check is it output of the submodule and it is user output wire
        Module* parentSrcModule = srcModuleGen->getMasterModulePtr()->getParentPtr();
        if ((parentSrcModule != nullptr) &&
            (parentSrcModule->getModuleGenPtr() == desModuleGen)){ //// check that src is submodule of this module
                Module* subModule = srcModuleGen->getMasterModulePtr();
                for (Wire* outputWire: subModule->getUserWiresByMarker(WMT_OUTPUT_MD)){
                    if (exactRealSrc == ((Operable*)outputWire)){
                        ///////// if it is match return realSrc
                        return realSrc;
                    }
                }
        }

        //// now it must be routed in some way

        std::vector<ModuleGen*> useInputAsModuleGen;
        std::vector<ModuleGen*> useOutputAsModuleGen;
        //// do routing until it match
        while (desModuleGen != srcModuleGen){
            int desDepth = desModuleGen->getDept();
            int srcDepth = srcModuleGen->getDept();
            //////// Iterate to upper module
            if (desDepth > srcDepth){
                useInputAsModuleGen.pushBack(desModuleGen);
                desModuleGen = desModuleGen->_master->getParentPtr()->getModuleGenPtr();
            }else if (desDepth < srcDepth){
                useOutputAsModuleGen.pushBack(srcModuleGen);
                srcModuleGen = srcModuleGen->_master->getParentPtr()->getModuleGenPtr();
            }else{
                useInputAsModuleGen.pushBack(desModuleGen);
                useOutputAsModuleGen.pushBack(srcModuleGen);
                desModuleGen = desModuleGen->_master->getParentPtr()->getModuleGenPtr();
                srcModuleGen = srcModuleGen->_master->getParentPtr()->getModuleGenPtr();
            }
        }
        useInputAsModuleGen.pushBack(desModuleGen);
        useOutputAsModuleGen.pushBack(srcModuleGen);


        /////////////////////////////////////
        /// inter wire
        /////////////////////////////////////
        ModuleGen* apogee    = *useInputAsModuleGen.rbegin();
        WireAuto*    interWire = nullptr;
        if (apogee->isThereAutoGenWire(exactRealSrc, WIRE_AUTO_GEN_INTER)){
            interWire = apogee->getAutoGenWire(exactRealSrc, WIRE_AUTO_GEN_INTER);
        }else{
            interWire = apogee->genAutoWireBase(
                exactRealSrc, exactRealSrc,
                "ABD_", WIRE_AUTO_GEN_INTER,
                false);
        }
        ////////////////////////////////////
        ///
        /// des series do it as input
        /// we assure the vector have at least one element in size
        /// //////////////////////////////////
        WireAuto* inputWire = interWire;
        for(int idx = ((int)useInputAsModuleGen.size()-2); idx >= 0; idx--){
            ModuleGen& curMdGen = *useInputAsModuleGen[idx];
            if (curMdGen.isThereAutoGenWire(exactRealSrc, WIRE_AUTO_GEN_INPUT)){
                inputWire = curMdGen.getAutoGenWire(exactRealSrc, WIRE_AUTO_GEN_INPUT);
                continue;
            }
            inputWire = curMdGen.genAutoWireBase(
                inputWire, exactRealSrc,
                "AIP_", WIRE_AUTO_GEN_INPUT,
                true);
        }


        ///////////////////////////////////
        ///
        ///src series do it as output
        ///
        ///
        ////////////////////////////////////
        Operable* outputWire = exactRealSrc;
        for(int idx = 0; idx < ((int)useOutputAsModuleGen.size()-1); idx++){
            ModuleGen& curMdGen = *useOutputAsModuleGen[idx];
            if (curMdGen.isThereAutoGenWire(exactRealSrc, WIRE_AUTO_GEN_OUTPUT)){
                outputWire = curMdGen.getAutoGenWire(exactRealSrc, WIRE_AUTO_GEN_OUTPUT);
                continue;
            }
            outputWire = curMdGen.genAutoWireBase(
                outputWire, exactRealSrc,
                "AOP_", WIRE_AUTO_GEN_OUTPUT,
                true);
        }
        interWire->connectTo(outputWire, true);
        //////////////////////////////

        if (realSrc->getOperableSlice() == exactRealSrc->getOperableSlice()){
            return inputWire;
        }
        ////// it is exact operable
        return &((*inputWire)
                (realSrc->getOperableSlice()));
    }

}
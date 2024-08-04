//
// Created by tanawin on 1/7/2024.
//

#include "model/hwComponent/module/module.h"

namespace kathryn{

    void ModuleGen::startRouteEle(){
        /////////// route io
        if (_master->isTopModule()){
            ////////////////////////// recruit first
            LogicGenBaseVec inputLogicGenBase;
            LogicGenBaseVec outputLogicGenBase;
            recruitLogicGenBase(inputLogicGenBase,  _ioWires[WIRE_IO_INPUT_GLOB ]);
            recruitLogicGenBase(outputLogicGenBase, _ioWires[WIRE_IO_OUTPUT_GLOB]);
            /////////////////////////// route dep for global io
            inputLogicGenBase.routeDepAll();
            outputLogicGenBase.routeDepAll();
        }
        /////////// module gen
        for(ModuleGen* mdGen: _subModulePool){
            mdGen->startRouteEle();
        }
        /////////// module gen
        _regPool        .routeDepAll();
        _wirePool       .routeDepAll();
        _exprPool       .routeDepAll();
        _nestPool       .routeDepAll();
        _valPool        .routeDepAll();
        _memBlockPool   .routeDepAll();
        _memBlockElePool.routeDepAll();
        /////////// no need to route io wire because it is the true level already
    }

    void ModuleGen::finalizeRouteEle() {
        for(ModuleGen* mdGen: _subModulePool){
            mdGen->finalizeRouteEle();
        }
        ////// recruit all io wire to logicgen base
        for (int i = 0; i < AMT_PHY_WIRE; i++){
            createAndRecruitLogicGenBase(_ioWirePools[i], _ioWires[i]);
        }
    }

    WireIo* ModuleGen::addAutoWireBase(
        Operable*          opr,      ///////// opr is exact opr
        Operable*          realSrc,  ///////// realSrc is exact opr too
        const std::string& wireName,
        WIRE_IO_TYPE       wireIoType,
        bool               connectTheWire){
            assert(opr     != nullptr);
            assert(realSrc != nullptr);
        assert((wireIoType == WIRE_IO_AUTO_INPUT) || (wireIoType == WIRE_IO_AUTO_OUTPUT));
        std::vector<WireIo*>&               ioVec = _ioWires[wireIoType];
        std::unordered_map<Operable*, int>& ioMap = _ioWireMaps[wireIoType];
        WireIo& newAddedWire = makeOprIoWire("addAutoWireBase_uninit",
                                              opr->getOperableSlice().getSize(),
                                              wireIoType
                                );
            ///////// addd update Event for only connection
            newAddedWire.buildHierarchy(_master);
            newAddedWire.createLogicGen();
            if (connectTheWire){ newAddedWire.connectTo(opr, true);}
            newAddedWire.setVarName(wireName +
                                    std::to_string(ioVec.size()) +
                                    "_" +
                                    realSrc->getLogicGenBase()->
                                    getIdent()->getVarName()
                                    );
            ioVec.push_back(&newAddedWire);
            ioMap.insert({realSrc, ioVec.size()-1});
            //////////////////////////////////////////////
            return &newAddedWire;
    }

    bool ModuleGen::isThereIoWire(Operable* realSrc, WIRE_IO_TYPE wireIoType){
        assert(wireIoType < AMT_PHY_WIRE);
        return _ioWireMaps[wireIoType].find(realSrc) != _ioWireMaps[wireIoType].end();
    }

    WireIo* ModuleGen::getIoWire(Operable* realSrc, WIRE_IO_TYPE wireIoType){
        std::vector<WireIo*>& ioVec = _ioWires[wireIoType];
        int linkOprIdx              = _ioWireMaps[wireIoType][realSrc];
        assert(linkOprIdx < ioVec.size());
        return ioVec[linkOprIdx];
    }

    /////////////////////////////// routing wire

    Operable* ModuleGen::routeSrcOprToThisModule(Operable* realSrc){
        assert(realSrc != nullptr);

        Operable* exactRealSrc = &realSrc->getExactOperable();

        ModuleGen* desModuleGen = this;
        ModuleGen* srcModuleGen = exactRealSrc->getLogicGenBase()->getModuleGen();

        ////// if it is same module then return that operable
        if (srcModuleGen == desModuleGen){
            return realSrc;
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
                useInputAsModuleGen.push_back(desModuleGen);
                desModuleGen = desModuleGen->_master->getParent()->getModuleGen();
            }else if (desDepth < srcDepth){
                useOutputAsModuleGen.push_back(srcModuleGen);
                srcModuleGen = srcModuleGen->_master->getParent()->getModuleGen();
            }else{
                useInputAsModuleGen.push_back(desModuleGen);
                useOutputAsModuleGen.push_back(srcModuleGen);
                desModuleGen = desModuleGen->_master->getParent()->getModuleGen();
                srcModuleGen = srcModuleGen->_master->getParent()->getModuleGen();
            }
        }
        useInputAsModuleGen.push_back(desModuleGen);
        useOutputAsModuleGen.push_back(srcModuleGen);


        /////////////////////////////////////
        /// inter wire
        /////////////////////////////////////
        ModuleGen* apogeeMod = *useInputAsModuleGen.rbegin();
        WireIo*    interWire = nullptr;
        if (apogeeMod->isThereIoWire(exactRealSrc, WIRE_IO_INTER)){
            interWire = apogeeMod->getIoWire(exactRealSrc, WIRE_IO_INTER);
        }else{
            interWire = apogeeMod->addAutoWireBase(exactRealSrc,
                                                       exactRealSrc,
                                                       "INTER_",
                                                       WIRE_IO_INTER,false);
        }
        ////////////////////////////////////
        ///
        /// des series do it as input
        /// we assure the vector have at least one element in size
        /// //////////////////////////////////
        WireIo* inputWire = interWire;
        for(int idx = ((int)useInputAsModuleGen.size()-2); idx >= 0; idx--){
            ModuleGen& curMdGen = *useInputAsModuleGen[idx];
            if (curMdGen.isThereIoWire(exactRealSrc, WIRE_IO_AUTO_INPUT)){
                inputWire = curMdGen.getIoWire(exactRealSrc, WIRE_IO_AUTO_INPUT);
                continue;
            }
            inputWire = curMdGen.addAutoWireBase(
                inputWire, exactRealSrc,
                "AI_", WIRE_IO_AUTO_INPUT, true);
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
            if (curMdGen.isThereIoWire(exactRealSrc, WIRE_IO_AUTO_OUTPUT)){
                outputWire = curMdGen.getIoWire(exactRealSrc, WIRE_IO_AUTO_OUTPUT);
                continue;
            }
            outputWire = curMdGen.addAutoWireBase(outputWire, exactRealSrc,
                "AO_", WIRE_IO_AUTO_OUTPUT, true);
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
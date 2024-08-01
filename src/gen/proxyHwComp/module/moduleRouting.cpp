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
            recruitLogicGenBase(inputLogicGenBase,_globalInputs);
            recruitLogicGenBase(outputLogicGenBase, _globalOutputs);
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
    }

    void ModuleGen::finalizeRouteEle() {
        for(ModuleGen* mdGen: _subModulePool){
            mdGen->finalizeRouteEle();
        }
        recruitLogicGenBase(_interWirePool, _interWires);
        recruitLogicGenBase(_autoInputWirePool, _autoInputWires);
        recruitLogicGenBase(_autoOutputWirePool, _autoOutputWires);
        recruitLogicGenBase(_globalInputPool, _globalInputs);
        recruitLogicGenBase(_globalOutputPool, _globalOutputs);
    }

    WireIoAuto* ModuleGen::addAutoWireBase(
        Operable* opr,      ///////// opr is exact opr
        Operable* realSrc,  ///////// realSrc is exact opr too
        std::vector<WireIoAuto*>& ioVec,
        std::unordered_map<Operable*, int>& ioMap,
        const std::string& wireName,
        WIRE_IO_TYPE wireIoType,
        bool connectTheWire){
            assert(opr     != nullptr);
            assert(realSrc != nullptr);
        WireIoAuto& newAddedWire = makeOprIoWire("addAutoWireBase_uninit",
                                              opr->getOperableSlice().getSize(),
                                              wireIoType
                                );

            ///////// addd update Event for only connection
            newAddedWire.buildHierarchy(_master);
            newAddedWire.createLogicGen();
            if (connectTheWire){
                newAddedWire.connectTo(opr, true);
            }

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

    /////////////////////////////// input wire

    WireIoAuto* ModuleGen::addAutoInputWire(Operable* opr,Operable* realSrc){
         return addAutoWireBase(opr,
             realSrc,
             _autoInputWires,
             _autoInputWireMap,
             "AIP_",
             WIRE_IO_AUTO_INPUT);
    }

    bool ModuleGen::checkIsThereAutoInputWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _autoInputWireMap.find(realSrc) != _autoInputWireMap.end();
    }

    WireIoAuto* ModuleGen::getAutoOutputWire(Operable* realSrc){
        assert(checkIsThereAutoOutputWire(realSrc));
        return _autoOutputWires[_autoOutputWireMap[realSrc]];
    }

    /////////////////////////////// output wire
    WireIoAuto* ModuleGen::addAutoOutputWire(Operable* opr,Operable* realSrc){
        return addAutoWireBase(opr,
            realSrc,
            _autoOutputWires,
            _autoOutputWireMap,
            "AOP_",
            WIRE_IO_AUTO_OUTPUT);
    }

    bool ModuleGen::checkIsThereAutoOutputWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _autoOutputWireMap.find(realSrc) != _autoOutputWireMap.end();
    }

    WireIoAuto* ModuleGen::getAutoInputWire(Operable* realSrc){
        assert(checkIsThereAutoInputWire(realSrc));
        return _autoInputWires[_autoInputWireMap[realSrc]];
    }

    ///////////////////////////////// inter wire

    WireIoAuto* ModuleGen::addAutoInterWire(Operable* realSrc){
        return addAutoWireBase(realSrc, realSrc,
        _interWires,
        _interWireMap,
        "ABD_",
        WIRE_IO_INTER,
        false);
    }

    bool ModuleGen::checkIsThereAutoInterWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _interWireMap.find(realSrc) != _interWireMap.end();
    }

    WireIoAuto* ModuleGen::getAutoInterWire(Operable* realSrc){
        assert(checkIsThereAutoInterWire(realSrc));
        return _interWires[_interWireMap[realSrc]];
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
        ModuleGen* apogee    = *useInputAsModuleGen.rbegin();
        WireIoAuto*    interWire = nullptr;
        if (apogee->checkIsThereAutoInterWire(exactRealSrc)){
            interWire = apogee->getAutoInterWire(exactRealSrc);
        }else{
            interWire = apogee->addAutoInterWire(exactRealSrc);
        }
        ////////////////////////////////////
        ///
        /// des series do it as input
        /// we assure the vector have at least one element in size
        /// //////////////////////////////////
        WireIoAuto* inputWire = interWire;
        for(int idx = ((int)useInputAsModuleGen.size()-2); idx >= 0; idx--){
            ModuleGen& curMdGen = *useInputAsModuleGen[idx];
            if (curMdGen.checkIsThereAutoInputWire(exactRealSrc)){
                inputWire = curMdGen.getAutoInputWire(exactRealSrc);
                continue;
            }
            inputWire = curMdGen.addAutoInputWire(inputWire, exactRealSrc);
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
            if (curMdGen.checkIsThereAutoOutputWire(exactRealSrc)){
                outputWire = curMdGen.getAutoOutputWire(exactRealSrc);
                continue;
            }
            outputWire = curMdGen.addAutoOutputWire(outputWire, exactRealSrc);
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
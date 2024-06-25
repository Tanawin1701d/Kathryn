//
// Created by tanawin on 20/6/2024.
//

#include "moduleGen.h"

#include "model/hwComponent/module/module.h"

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
    void ModuleGen::recruitLogicGenBase(std::vector<LogicGenBase*>& des,
                                 std::vector<T*>& srcs){
        for(T* src: srcs){
            LogicGenBase* logicGenBase = src->getLogicGen();
            assert(logicGenBase != nullptr);
            des.push_back(logicGenBase);
        }
    }

    template<typename T>
    void ModuleGen::createAndRecruitLogicGenBase(
        std::vector<LogicGenBase*>& des,
        std::vector<T*>& srcs){
        createLogicGenBase(srcs);
        recruitLogicGenBase(des, srcs);
    }

    void ModuleGen::doOpLogicGenVec(
    std::vector<LogicGenBase*>& src,
    void (LogicGenBase::*func)()
    ){
        for (LogicGenBase* lgb: src){
            lgb->*func();
        }
    }

    void
    doOpLogicGenVec(std::vector<std::string>&   result,
                        std::vector<LogicGenBase*>& src,
                        std::string (LogicGenBase::*func)()){
        for (LogicGenBase* lgb: src){
            result.push_back(lgb->*func());
        }
    }

    void ModuleGen::doOpLogicGenAndWrite(
        std::vector<LogicGenBase*>& src,
        std::string (LogicGenBase::*func)(),
        FileWriterBase* fileWriter,
        const std::string& seperator
        ){
        //////
        /// generate string and immediatry write to file
        ///
        bool isFirst = true;
        for (LogicGenBase* logicEle: src){
            if (!isFirst){
                fileWriter->addData(",");
            }
            fileWriter->addData(logicEle->*func());
            fileWriter->addData("\n");
            isFirst = false;
        }
    }


    void ModuleGen::startInitEle(){

        depthFromGlobalModule = _master->getModuleGen()->getDept() + 1;

        //////// init the sub module elements first
        for (Module* subModule: _master->getUserSubModules()){
            subModule->createModuleGen();
            subModule->getModuleGen()->startInitEle();
            _subModulePool.push_back(subModule->getModuleGen());
        }

        //////// init all logic element wo test
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

    }

    void ModuleGen::startRouteEle(){

        /////////// module gen
        for(ModuleGen* mdGen: _subModulePool){
            mdGen->startRouteEle();
        }
        /////////// module gen
        doOpLogicGenVec(_regPool        , LogicGenBase::routeDep);
        doOpLogicGenVec(_wirePool       , LogicGenBase::routeDep);
        doOpLogicGenVec(_exprPool       , LogicGenBase::routeDep);
        doOpLogicGenVec(_nestPool       , LogicGenBase::routeDep);
        doOpLogicGenVec(_valPool        , LogicGenBase::routeDep);
        doOpLogicGenVec(_memBlockPool   , LogicGenBase::routeDep);
        doOpLogicGenVec(_memBlockElePool, LogicGenBase::routeDep);

    }

    void ModuleGen::startCmpModule(ModuleGen* rhsMdg){
        assert(false);
    }

    void ModuleGen::startWriteFile(FileWriterBase* fileWriter){
        std::vector<LogicGenBase*> inputVec;
        std::vector<LogicGenBase*> outputVec;
        std::vector<LogicGenBase*> bridgeVec;
        recruitLogicGenBase (inputVec, _autoInputWires);
        recruitLogicGenBase(outputVec, _autoOutputWires);
        recruitLogicGenBase(bridgeVec, _interWires);

        fileWriter->addData("module ");
        fileWriter->addData(_master->getGlobalName());
        fileWriter->addData("(\n");
        //////// declare input element
        doOpLogicGenAndWrite(inputVec, LogicGenBase::decIo, fileWriter);
        if (!inputVec.empty()){
            fileWriter->addData(",");
        }
        //////// declare output element
        doOpLogicGenAndWrite(outputVec, LogicGenBase::decIo, fileWriter);

        fileWriter->addData(");\n");

        //////// todo do it with sub module

        //////// declare variable initiation
        doOpLogicGenAndWrite(_regPool        , LogicGenBase::decVariable, fileWriter);
        doOpLogicGenAndWrite(_wirePool       , LogicGenBase::decVariable, fileWriter);
        doOpLogicGenAndWrite(_exprPool       , LogicGenBase::decVariable, fileWriter);
        doOpLogicGenAndWrite(_nestPool       , LogicGenBase::decVariable, fileWriter);
        doOpLogicGenAndWrite(_valPool        , LogicGenBase::decVariable, fileWriter);
        doOpLogicGenAndWrite(_memBlockPool   , LogicGenBase::decVariable, fileWriter);
        doOpLogicGenAndWrite(_memBlockElePool, LogicGenBase::decVariable, fileWriter);
        doOpLogicGenAndWrite(bridgeVec       , LogicGenBase::decVariable, fileWriter);
        /////////////////////// declare variable for submodule connenection
        ////////////////////////////////// there is no need to do op on this wire due to
        ////////////////////////////////// sub module is driver not us
        std::vector<LogicGenBase*> subModuleOutputRepresent;
        for (ModuleGen* subMdGen: _subModulePool){
            recruitLogicGenBase(subModuleOutputRepresent,
                                subMdGen->_autoOutputWires
            );
        }
        doOpLogicGenAndWrite(subModuleOutputRepresent, LogicGenBase::decVariable, fileWriter);


        //////// declare operation initiation
        doOpLogicGenAndWrite(_regPool        , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_wirePool       , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_exprPool       , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_nestPool       , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_valPool        , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_memBlockPool   , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_memBlockElePool, LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(bridgeVec       , LogicGenBase::decOp, fileWriter);
        ////////// declare submodule connectivity
        for (ModuleGen* subMdGen: _subModulePool){
            subMdGen->getSubModuleDec(subMdGen);
        }

        //////// end module

        fileWriter->addData("endmodule\n");
    }


    WireIo* ModuleGen::addAutoWireBase(
        Operable* opr,      ///////// opr is exact opr
        Operable* realSrc,  ///////// realSrc is exact opr too
        std::vector<WireIo*>& ioVec,
        std::unordered_map<Operable*, int>& ioMap,
        const std::string& wireName,
        WIRE_IO_TYPE wireIoType,
        bool connectTheWire){
            assert(opr     != nullptr);
            assert(realSrc != nullptr);
            WireIo& newAddedWire =
                _make<WireIo>(wireName +
                              std::to_string(ioVec.size()),
                              false,
                              opr->getOperableSlice().getSize(),
                              wireIoType
                              );
            ///////// addd update Event for only connection
            if (connectTheWire){
                newAddedWire.connectTo(opr);
            }
            newAddedWire.buildHierarchy(_master);
            ioVec.push_back(&newAddedWire);
            ioMap.insert({realSrc, ioVec.size()-1});
            //////////////////////////////////////////////
            return &newAddedWire;
    }

    /////////////////////////////// input wire

    WireIo* ModuleGen::addAutoInputWire(Operable* opr,Operable* realSrc){
         return addAutoWireBase(opr,
             realSrc,
             _autoInputWires,
             _autoInputWireMap,
             "autoInputWire_",
             WIRE_IO_INPUT);
    }

    bool ModuleGen::checkIsThereAutoInputWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _autoInputWireMap.find(realSrc) != _autoInputWireMap.end();
    }

    WireIo* ModuleGen::getAutoOutputWire(Operable* realSrc){
        assert(checkIsThereAutoOutputWire(realSrc));
        return _autoOutputWires[_autoOutputWireMap[realSrc]];
    }

    /////////////////////////////// output wire
    WireIo* ModuleGen::addAutoOutputWire(Operable* opr,Operable* realSrc){
        return addAutoWireBase(opr,
            realSrc,
            _autoOutputWires,
            _autoOutputWireMap,
            "autoOutputWire_",
            WIRE_IO_OUTPUT);
    }

    bool ModuleGen::checkIsThereAutoOutputWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _autoOutputWireMap.find(realSrc) != _autoOutputWireMap.end();
    }

    WireIo* ModuleGen::getAutoInputWire(Operable* realSrc){
        assert(checkIsThereAutoInputWire(realSrc));
        return _autoInputWires[_autoInputWireMap[realSrc]];
    }

    ///////////////////////////////// inter wire

    WireIo* ModuleGen::addAutoInterWire(Operable* realSrc){
        return addAutoWireBase(realSrc, realSrc,
        _interWires,
        _interWireMap,
        "autoInterWire_",
        WIRE_IO_INTER,
        false);
    }

    bool ModuleGen::checkIsThereAutoInterWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _interWireMap.find(realSrc) != _interWireMap.end();
    }

    WireIo* ModuleGen::getAutoInterWire(Operable* realSrc){
        assert(checkIsThereAutoInterWire(realSrc));
        return _interWires[_interWireMap[realSrc]];
    }

    /////////////////////////////// routing wire

    Operable* ModuleGen::routeSrcOprToThisModule(Operable* realSrc){
        assert(realSrc != nullptr);

        Operable* exactRealSrc = &realSrc->getExactOperable();

        ModuleGen* desModuleGen = this;
        ModuleGen* srcModuleGen = exactRealSrc->getLogicGenBase()->getModuleGen();

        if (srcModuleGen == desModuleGen){
            return exactRealSrc;
        }

        std::vector<ModuleGen*> useInputAsModuleGen;
        std::vector<ModuleGen*> useOutputAsModuleGen;
        //////////////////////////////
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
        WireIo*      interWire = nullptr;
        if (checkIsThereAutoInterWire(exactRealSrc)){
            interWire = apogee->getAutoInterWire(exactRealSrc);
        }else{
            interWire = apogee->addAutoInterWire(exactRealSrc);
        }
        ////////////////////////////////////
        ///
        /// des series do it as input
        /// we assure the vector have at least one element in size
        /// //////////////////////////////////
        Operable* inputWire = interWire;
        for(int idx = ((int)useInputAsModuleGen.size()-1); idx >= 0; idx--){
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
        interWire->connectTo(outputWire);
        //////////////////////////////

        if (realSrc->getOperableSlice() == exactRealSrc->getOperableSlice()){
            return useInputAsModuleGen[0]->getAutoInputWire(exactRealSrc);
        }
        ////// it is exact operable
        return useInputAsModuleGen[0]
        ->getAutoInputWire(exactRealSrc)(realSrc->getOperableSlice());
    }

    //////////////////////////// get module dec as sub
    ///
    std::string ModuleGen::getSubModuleDec(ModuleGen* subMdGen){
        assert(subMdGen != nullptr);

        std::vector<std::string> retStrs;
        std::string result;

        result += subMdGen->getOpr() + "  ";
        result += subMdGen->getOpr();
        result += "(\n";

        ////////////////// declare input and output
        std::vector<LogicGenBase*> inputGenEle;
        std::vector<LogicGenBase*> outputGenEle;
        recruitLogicGenBase(inputGenEle, _autoInputWires);
        recruitLogicGenBase(outputGenEle, _autoOutputWires);


        doOpLogicGenVec(retStrs, inputGenEle , &LogicGenBase::getOpr);
        doOpLogicGenVec(retStrs, outputGenEle, &LogicGenBase::getOpr);

        bool isFirst = true;

        for (const std::string& retStr: retStrs){
            if (!isFirst){
                result += ",\n";
            }
            result += retStr;
            isFirst = false;
        }

        result += ");\n";
        return result;

    }

    std::string ModuleGen::getOpr(){
        return _master->getGlobalName();
    }

}

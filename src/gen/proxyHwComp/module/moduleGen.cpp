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
    void ModuleGen::recruitLogicGenBase(LogicGenBaseVec& des,
                                 std::vector<T*>& srcs){
        for(T* src: srcs){
            LogicGenBase* logicGenBase = src->getLogicGen();
            assert(logicGenBase != nullptr);
            des.push_back(logicGenBase);
        }
    }

    template<typename T>
    void ModuleGen::createAndRecruitLogicGenBase(
        LogicGenBaseVec& des,
        std::vector<T*>& srcs){
        createLogicGenBase(srcs);
        recruitLogicGenBase(des, srcs);
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

    void ModuleGen::startCmpModule(ModuleGen* rhsMdg){
        assert(false);
    }

    void ModuleGen::startWriteFile(FileWriterBase* fileWriter){
        LogicGenBaseVec globInputVec;
        LogicGenBaseVec globOutputVec;
        LogicGenBaseVec inputVec;
        LogicGenBaseVec outputVec;
        LogicGenBaseVec bridgeVec;
        LogicGenBaseVec subModuleOutputRepresent;
        LogicGenBaseVec subModuleInputRepresent;

        ///////////// recruit io /////////////////////////////////////
        recruitLogicGenBase(globInputVec , _globalInputs);
        recruitLogicGenBase(globOutputVec, _globalOutputs);
        recruitLogicGenBase(inputVec     , _autoInputWires);
        recruitLogicGenBase(outputVec    , _autoOutputWires);
        recruitLogicGenBase(bridgeVec    , _interWires);
        for (ModuleGen* subMdGen: _subModulePool){
            recruitLogicGenBase(subModuleOutputRepresent,
                                subMdGen->_autoOutputWires);
            recruitLogicGenBase(subModuleInputRepresent,
                                subMdGen->_autoInputWires);
        }
        ///////////////////////////////////////////////////////////

        /**
         *
         *   module io declaration
         *
         */
        fileWriter->addData("//////////////////////////////////////////////////////////////////////////////\n"
                            "//////////////////// MODULE DEC ////////////////////////////////////////////////\n"
                            "////////////////////////////////////////////////////////////////////////////////\n"
                            );
        fileWriter->addData("module ");
        fileWriter->addData(_master->getGlobalName() + "(\n");

        //////// declare input/output element
        std::vector<std::string> ioVec = getIoDec(inputVec,outputVec,globInputVec,globOutputVec);
        ioVec.emplace_back("input wire clk");
        writeGenVec(ioVec, fileWriter, ",\n");
        fileWriter->addData("\n);\n");

        /***                          |   wire declaration   |   wire operation   |
         * ------------------------------------------------------------------------
         * glob/auto input  wire ---->           no          |         no
         * glob/auto output wire ---->           no          |        yes
         * submodule input wire  ---->          yes          |        yes
         * submodule output wire ---->          yes          |         no
         * ------------------------------------------------------------------------
         * bridge wire           ---->          yes          |        yes
         * ------------------------------------------------------------------------
         */

        /*
         * declare variable initiation
         *
         */
        fileWriter->addData("////regDecVar\n");
        writeGenVec(_regPool.getDecVars()                , fileWriter, "\n"); fileWriter->addData("\n////wireDecVar\n");
        writeGenVec(_wirePool.getDecVars()               , fileWriter, "\n"); fileWriter->addData("\n////_exprPool\n");
        writeGenVec(_exprPool.getDecVars()               , fileWriter, "\n"); fileWriter->addData("\n////_nestPool\n");
        writeGenVec(_nestPool.getDecVars()               , fileWriter, "\n"); fileWriter->addData("\n////_valPool\n");
        writeGenVec(_valPool.getDecVars()                , fileWriter, "\n"); fileWriter->addData("\n////_memBlockPool\n");
        writeGenVec(_memBlockPool.getDecVars()           , fileWriter, "\n"); fileWriter->addData("\n////_memBlockElePool\n");
        writeGenVec(_memBlockElePool.getDecVars()        , fileWriter, "\n"); fileWriter->addData("\n////input of submodule\n");
        writeGenVec(subModuleInputRepresent.getDecVars() , fileWriter, "\n"); fileWriter->addData("\n////output of submodule\n");
        writeGenVec(subModuleOutputRepresent.getDecVars(), fileWriter, "\n"); fileWriter->addData("\n////bridgeVec\n");
        writeGenVec(bridgeVec.getDecVars()               , fileWriter, "\n");

        fileWriter->addData("///////////////////////////////////////////////\n"
                            "//////////////////// operation///////////////////\n"
                            "/////////////////////////////////////////////////\n"
                            );



        fileWriter->addData("\n///regOp\n");
        writeGenVec(_regPool.getDecOps()                 , fileWriter, "\n"); fileWriter->addData("\n///_wirePoolOp\n");
        writeGenVec(_wirePool.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///_exprPoolOp\n");
        writeGenVec(_exprPool.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///_nestPoolOp\n");
        writeGenVec(_nestPool.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///_valPoolOp\n");
        writeGenVec(_valPool.getDecOps()                 , fileWriter, "\n"); fileWriter->addData("\n///_memBlockPoolOp\n");
        writeGenVec(_memBlockPool.getDecOps()            , fileWriter, "\n"); fileWriter->addData("\n///_memBlockElePoolOp\n");
        writeGenVec(_memBlockElePool.getDecOps()         , fileWriter, "\n"); fileWriter->addData("\n///outputVecOp\n");
        writeGenVec(outputVec.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///outputVecOp global\n");
        writeGenVec(globOutputVec.getDecOps()            , fileWriter, "\n"); fileWriter->addData("\n////input of submodule\n");
        writeGenVec(subModuleInputRepresent.getDecOps()  , fileWriter, "\n"); fileWriter->addData("\n///bridgeVecOp\n");
        writeGenVec(bridgeVec.getDecOps()                , fileWriter, "\n");
        ////////// declare submodule connectivity
        fileWriter->addData("/// sub module declaration\n");
        for (ModuleGen* subMdGen: _subModulePool){
            fileWriter->addData(subMdGen->getSubModuleDec(subMdGen));
        }
        //////// end module
        fileWriter->addData("\nendmodule\n\n");

        for (ModuleGen* subMdGen: _subModulePool){
            subMdGen->startWriteFile(fileWriter);
        }
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
            newAddedWire.buildHierarchy(_master);
            newAddedWire.createLogicGen();
            if (connectTheWire){
                newAddedWire.connectTo(opr, true);
            }
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
        WireIo*    interWire = nullptr;
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
        WireIo* inputWire = interWire;
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

    std::vector<std::string> ModuleGen::getIoDec(
        const LogicGenBaseVec& inputVec,
        const LogicGenBaseVec& outputVec,
        const LogicGenBaseVec& globInputVec,
        const LogicGenBaseVec& globOutputVec){

        std::vector<std::string> result;

        for (LogicGenBase* lgb: inputVec)     {result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: outputVec)    {result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: globInputVec) {result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: globOutputVec){result.push_back(lgb->decIo());}

        return result;
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
        LogicGenBaseVec inputGenEle;
        LogicGenBaseVec outputGenEle;
        recruitLogicGenBase(inputGenEle, _autoInputWires);
        recruitLogicGenBase(outputGenEle, _autoOutputWires);

        for (const std::string& inputStr : inputGenEle.getOprs()){
            retStrs.push_back(inputStr);
        }
        for (const std::string& outputStr: outputGenEle.getOprs()){
            retStrs.push_back(outputStr);
        }
        retStrs.push_back("clk");

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

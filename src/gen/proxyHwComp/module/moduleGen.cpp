//
// Created by tanawin on 20/6/2024.
//

#include "moduleGen.h"

#include <model/hwComponent/module/module.h>

#include "gen/proxyHwComp/abstract/logicGenBase.h"

namespace kathryn{

    template<typename T>
    void ModuleGen::recruitLogicGenBase(std::vector<LogicGenBase*>& des,
                                 std::vector<T*>& srcs){
        for(T* src: srcs){
            LogicGenBase* logicGenBase = src->getLogicGen();
            assert(logicGenBase != nullptr);
            des.push_back(logicGenBase);
        }
    }

    void ModuleGen::doOpLogicGenVec(
    std::vector<LogicGenBase*>& des,
    void (LogicGenBase::*func)()
    ){

        for (LogicGenBase* lgb: des){
            lgb->*func();
        }
    }

    // std::vector<std::string> ModuleGen::doOpLogicGenVec(
    //     std::vector<LogicGenBase*>& src,
    //     std::string (LogicGenBase::*func)()
    // ){
    //     std::vector<std::string> result;
    //     for(LogicGenBase* lgb: src){
    //         result.push_back(lgb->*func());
    //     }
    //     return result;
    // }
    //
    // void ModuleGen::writeThisVector(std::vector<std::string>& writeData,
    //                                 FileWriterBase* fileWriter
    // ){
    //     assert(fileWriter != nullptr);
    //     for(std::string& dayta: writeData){
    //         fileWriter->addData(dayta);
    //     }
    // }

    void ModuleGen::doOpLogicGenAndWrite(
        std::vector<LogicGenBase*>& src,
        std::string (LogicGenBase::*func)(),
        FileWriterBase* fileWriter
        ){
        //////
        /// generate string and immediatry write to file
        ///
        for (LogicGenBase* logicEle: src){
            fileWriter->addData(logicEle->*func());
            fileWriter->addData("\n");
        }

    }


    void ModuleGen::startInitEle(){

        for (int spIdx = 0; spIdx < SP_CNT_REG; spIdx++){
            recruitLogicGenBase(
                _regPool,
                _master->getSpRegs((SP_REG_TYPE)spIdx));
        }
        recruitLogicGenBase(_regPool, _master->getUserRegs());
        recruitLogicGenBase(_wirePool,_master->getUserWires());
        recruitLogicGenBase(_exprPool,_master->getUserExpressions());
        recruitLogicGenBase(_nestPool,_master->getUserNests());
        recruitLogicGenBase(_valPool, _master->getUserVals());

        recruitLogicGenBase(_memBlockPool, _master->getUserMemBlks());

        for (MemBlock* memBlock: _master->getUserMemBlks()){
            recruitLogicGenBase(_memBlockElePool, memBlock->getMemBlockAgents());
        }

        for (Module* subModule: _master->getUserSubModules()){
            _subModulePool.push_back(subModule->getModuleGen());
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

        fileWriter->addData("module ");
        fileWriter->addData(_master->getGlobalName());
        fileWriter->addData("(\n");
        //////// declare input element

        recruitLogicGenBase(inputVec, _autoInputWires);
        doOpLogicGenAndWrite(inputVec, LogicGenBase::decIo, fileWriter);
        //////// declare output element

        recruitLogicGenBase(outputVec, _autoOutputWires);
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

        recruitLogicGenBase(bridgeVec, _interWires);
        doOpLogicGenAndWrite(_memBlockElePool, LogicGenBase::decVariable, fileWriter);
        /////////////////////// declare variable for submodule
        std::vector<LogicGenBase*> outputOfSubModulePool;
        for (ModuleGen* subMdGen: _subModulePool){
            recruitLogicGenBase(outputOfSubModulePool,
                                subMdGen->_autoOutputWires
            );
        }
        doOpLogicGenAndWrite(outputOfSubModulePool, LogicGenBase::decVariable, fileWriter);


        //////// declare operation initiation
        doOpLogicGenAndWrite(_regPool        , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_wirePool       , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_exprPool       , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_nestPool       , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_valPool        , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_memBlockPool   , LogicGenBase::decOp, fileWriter);
        doOpLogicGenAndWrite(_memBlockElePool, LogicGenBase::decOp, fileWriter);

        recruitLogicGenBase(bridgeVec, _interWires);
        doOpLogicGenAndWrite(_memBlockElePool, LogicGenBase::decOp, fileWriter);

        ////////// declare submodule connectivity
        for (ModuleGen* subMdGen: _subModulePool){
            subMdGen->getSubModuleDec(subMdGen);
        }

        //////// end module

        fileWriter->addData("endmodule\n");
    }


    Wire* ModuleGen::addAutoWireBase(
        Operable* opr,
        Operable* realSrc,
        std::vector<Wire*>& ioVec,
        std::unordered_map<Operable*, int>& ioMap,
        const std::string& wireName){
        assert(opr     != nullptr);
        assert(realSrc != nullptr);
        Wire& newAddedWire =
            _make<Wire>(wireName +
            std::to_string(ioVec.size()),
            false,
            opr->getOperableSlice().getSize(),
            false
            );
        ///////// addd update Event for only connection
        std::vector<UpdateEvent*>& updatePool =
            newAddedWire.getUpdateMeta();

        auto* updateEvent =
            new UpdateEvent(
                {nullptr,
                    nullptr,
                    opr,
                    {0, opr->getOperableSlice().getSize()}});
        updatePool.push_back(updateEvent);
        ioVec.push_back(&newAddedWire);
        ////////////////////////////////////////////////
        ioMap.insert({realSrc, ioVec.size()-1});
        //////////////////////////////////////////////
        return &newAddedWire;
    }

    /////////////////////////////// input wire

    Operable* ModuleGen::addAutoInputWire(Operable* opr,Operable* realSrc){
         return addAutoWireBase(opr,
             realSrc,
             _autoInputWires,
             _autoInputWireMap,
             "autoInputWire_");
    }

    bool ModuleGen::checkIsThereAutoInputWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _autoInputWireMap.find(realSrc) != _autoInputWireMap.end();
    }

    Operable* ModuleGen::getAutoOutputWire(Operable* realSrc){
        assert(checkIsThereAutoOutputWire(realSrc));
        return _autoOutputWires[_autoOutputWireMap[realSrc]];
    }

    /////////////////////////////// output wire
    Operable* ModuleGen::addAutoOutputWire(Operable* opr,Operable* realSrc){
        return addAutoWireBase(opr,
            realSrc,
            _autoOutputWires,
            _autoOutputWireMap,
            "autoOutputWire_");
    }

    bool ModuleGen::checkIsThereAutoOutputWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _autoOutputWireMap.find(realSrc) != _autoOutputWireMap.end();
    }

    Operable* ModuleGen::getAutoInputWire(Operable* realSrc){
        assert(checkIsThereAutoInputWire(realSrc));
        return _autoInputWires[_autoInputWireMap[realSrc]];
    }

    ///////////////////////////////// inter wire

    Wire* ModuleGen::addAutoIterWire(Operable* realSrc){
        return addAutoWireBase(realSrc, realSrc,
        _interWires,
        _interWireMap,
        "autoInterWire_");
    }

    bool ModuleGen::checkIsThereAutoInterWire(Operable* realSrc){
        assert(realSrc != nullptr);
        return _interWireMap.find(realSrc) != _interWireMap.end();
    }

    Wire* ModuleGen::getAutoInterWire(Operable* realSrc){
        assert(checkIsThereAutoInterWire(realSrc));
        return _interWires[_interWireMap[realSrc]];
    }

    /////////////////////////////// routing wire

    Operable* ModuleGen::routeSrcOprToThisModule(Operable* realSrc){
        assert(realSrc != nullptr);

        ModuleGen* desModuleGen   = this;
        ModuleGen* srcModuleGen = realSrc->getLogicGenBase()->getModuleGen();

        if (srcModuleGen == desModuleGen){
            return realSrc;
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

        ModuleGen* apogee    = *useInputAsModuleGen.rbegin();
        Wire*      interWire = nullptr;
        if (checkIsThereAutoInterWire(realSrc)){
            interWire = apogee->getAutoInterWire(realSrc);
        }else{
            interWire = apogee->addAutoIterWire(realSrc);
        }
        ////////////////////////////////////
        ///
        /// des series do it as input
        /// we assure the vector have at least one element in size
        /// //////////////////////////////////
        Operable* inputWire = interWire;
        for(int idx = ((int)useInputAsModuleGen.size()-1); idx >= 0; idx--){
            ModuleGen& curMdGen = *useInputAsModuleGen[idx];
            if (curMdGen.checkIsThereAutoInputWire(realSrc)){
                inputWire = curMdGen.getAutoInputWire(realSrc);
                break;
            }
            inputWire = curMdGen.addAutoInputWire(inputWire, realSrc);
        }


        ///////////////////////////////////
        ///
        ///src series do it as output
        ///
        ///
        ////////////////////////////////////
        Operable* outputWire = realSrc;
        for(int idx = 0; idx < ((int)useOutputAsModuleGen.size()-1); idx--){
            ModuleGen& curMdGen = *useOutputAsModuleGen[idx];
            if (curMdGen.checkIsThereAutoOutputWire(realSrc)){
                outputWire = curMdGen.getAutoOutputWire(realSrc);
                break;
            }
            outputWire = curMdGen.addAutoOutputWire(outputWire, realSrc);
        }

        std::vector<UpdateEvent*>& udEvents= interWire->getUpdateMeta();
        udEvents.push_back(new UpdateEvent({
            nullptr,
            nullptr,
            outputWire,
            {0, interWire->getOperableSlice().getSize()}
        }));


        return useInputAsModuleGen[0]->getAutoInputWire(realSrc);
    }

    //////////////////////////// get module dec as sub
    ///
    std::string ModuleGen::getSubModuleDec(ModuleGen* subMdGen){
        assert(subMdGen != nullptr);

        std::vector<std::string> retStrs;

        for (Wire* inputWire: subMdGen->_autoInputWires){
            auto inputWireMeta = inputWire->getUpdateMeta();
            assert(inputWireMeta.size() == 1);
            LogicGenBase* curModConOpr = inputWireMeta[0]->srcUpdateValue->getLogicGenBase();
            retStrs.push_back(
                curModConOpr->getOpr(inputWireMeta[0]->srcUpdateValue->getOperableSlice()));
        }

        for (Wire* outputConnectWire: subMdGen->_autoOutputWires){
            auto lgb = outputConnectWire->getLogicGenBase(); ///// logic gen base

            retStrs.push_back(
                lgb->getOpr(outputConnectWire->getOperableSlice())
            );
        }

        std::string result;
        bool isFirst = true;

        for (int i = 0; i < ((int)retStrs.size())-1; i++){
            if (!isFirst){
                result += ",\n";
            }
            result += retStrs[i];
            isFirst = false;
        }
        return result;
    }








}

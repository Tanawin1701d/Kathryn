//
// Created by tanawin on 20/6/2024.
//

#include "genController.h"
#include "model/controller/controller.h"
#include "model/hwComponent/abstract/globPool.h"


namespace kathryn{

    void GenController::initEnv(PARAM& param){
        _desVerilogPath = param[_desPathParamPrefix];
        _verilogWriter  = new FileWriterBase(_desVerilogPath);
        _masterModule   = getGlobalModulePtr();
    }

    void GenController::initEle(){

        _masterModule->createModuleGen();
        _masterModule->setTopModule();
        _masterModuleGen = _masterModule->getModuleGen();
        _masterModuleGen->startInitEle();
        initGlobEle(true); ///// init input
        initGlobEle(false); ////// init output

    }

    void GenController::start(){
        initEle();
        routeIo();
        genCefAll();
        recruitModToGenSystem();
        generateEveryModule();
    }

    void GenController::routeIo(){
        //////// it is recursive function
        _masterModuleGen->startRouteEle();
        _masterModuleGen->finalizeRouteEle();
    }

    void GenController::genCefAll(){
        _masterModuleGen->genCerfAll(0);
    }




    void GenController::recruitModToGenSystem() {
        //////// buttom up only
        _masterModuleGen->startPutToGenSystem(&_genStructure);
    }

    void GenController::generateEveryModule(){

        for (ModuleGen* masterModuleGen:
        _genStructure.getAllMasterModuleGen()){
            assert(masterModuleGen != nullptr);
            masterModuleGen->
            startWriteFile(_verilogWriter, &_genStructure);
        }
    }

    void GenController::reset(){
        if (_verilogWriter != nullptr){
            _verilogWriter->flush();
        }
        delete _verilogWriter;
        _verilogWriter = nullptr;
        _genStructure.reset();
    }

    void GenController::clean(){
        reset();
    }

    GenController* genCtrl = nullptr;

    GenController* getGenController(){

        /////////// gen controller
        if (genCtrl == nullptr){
            genCtrl = new GenController();
        }
        return genCtrl;
    }




    ///////////////////////// inside element

    void GenController::initGlobEle(bool isInput){
        assert(_masterModule != nullptr);
        assert(_masterModuleGen != nullptr);
        std::vector<WireIo*>& desSaveIo = isInput ? _masterModuleGen->getGlobalInputs()
                                                  : _masterModuleGen->getGlobalOutputs();
        ///////
        /////// input/output
        ///////

        /////////// get it from global pool
        for(GlobIoItf* srcToBeGlobIo: getGlobPool(isInput)){
            assert(srcToBeGlobIo != nullptr);
            Operable* originOpr = srcToBeGlobIo->getOprFromGlobIo();
            assert(originOpr != nullptr);
            //////// the host wire require inserting
            auto& inputIo = makeOprIoWire(
                    srcToBeGlobIo->getGlobIoName(),
                    originOpr->getOperableSlice().getSize(),
                    isInput ? WIRE_IO_INPUT_GLOB : WIRE_IO_OUTPUT_GLOB
                );
            inputIo.buildHierarchy(_masterModule);
            inputIo.createLogicGen();
            /////// connect
            if(isInput){
                srcToBeGlobIo->connectToThisIo(&inputIo);
            }else{
                inputIo.connectTo(originOpr, false); /////// it may needs to be route
            }
            desSaveIo.push_back(&inputIo);
        }

    }




}

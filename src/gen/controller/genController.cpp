//
// Created by tanawin on 20/6/2024.
//

#include "genController.h"
#include "model/controller/controller.h"
#include "model/hwComponent/abstract/globPool.h"
#include "model/hwComponent/wire/wireAuto.h"


namespace kathryn{


    GenController::GenController():
    PROJECT_PATH(KATHRYN_PROJECT_DIR){}

    void GenController::initEnv(PARAM& param){
        _desVerilogFolder      = param[_desVerilogFolderParamPrefix     ];
        _desVerilogTopFileName = param[_desVerilogTopFileNameParamPrefix];
        _desVerilogTopModName  = param[_desVerilogTopModNameParamPrefix ];
        _extractMulFile =
            (param[_desVerilogTopModNameParamPrefix ] == "true");
        _desSynName     = param[_desSynthesisPrefix];

        _writerGroup.setPrefixFolder(_desVerilogFolder);
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
        generateEveryModule();
    }

    void GenController::routeIo(){
        //////// it is recursive function
        _masterModuleGen->startRouteEle();
        _masterModuleGen->finalizeRouteEle();
    }

    void GenController::generateEveryModule(){

        FileWriterBase* topWriter = _writerGroup.createNewFile(_desVerilogTopFileName + _file_suffix);

        _masterModuleGen->startWriteFileMaster(_extractMulFile, topWriter, &_writerGroup, true, _desVerilogTopModName);

    }

    void GenController::startSynthesis(){

        assert(!_desSynName.empty());
        _writerGroup.flushAll();
        if (!_extractMulFile){
            std::cout << "we did not afford synthesis runner for multiple file yet";
            assert(false);
        }
        std::string compileComand =
            pathToVivadoLaunch + " " + _desSynName + " " +
                _desVerilogFolder + "/" + _desVerilogTopFileName;
        int compileComd = system(compileComand.c_str());
        std::cout << "synthesis result: " << compileComd << std::endl;
    }

    void GenController::reset(){
        _writerGroup.clean();
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
        std::vector<WireAuto*>& desSaveIo = isInput ? _masterModuleGen->_genWires[WIRE_AUTO_GEN_GLOB_INPUT]
                                                    : _masterModuleGen->_genWires[WIRE_AUTO_GEN_GLOB_OUTPUT];
        ///////
        /////// input/output
        ///////

        /////////// get it from global pool
        for(WireMarker* srcToBeGlobIo: getGlobPool(isInput)){
            assert(srcToBeGlobIo != nullptr);
            Operable* originOpr = srcToBeGlobIo->getOprFromGlobIo();
            assert(originOpr != nullptr);
            //////// the host wire require inserting
            auto& inputIo = makeOprIoWire(
                    srcToBeGlobIo->getGlobIoName(),
                    originOpr->getOperableSlice().getSize(),
                    isInput ? WIRE_AUTO_GEN_GLOB_INPUT : WIRE_AUTO_GEN_GLOB_OUTPUT
                );
            inputIo.buildHierarchy(_masterModule);
            inputIo.createLogicGen();
            /////// connect
            if(isInput){

                UpdateEventBasic* connectEvent =
                    createUEHelper(
                        &inputIo,
                        inputIo.getOperableSlice(),
                        DEFAULT_UE_PRI_MIN,
                        CM_CLK_FREE,
                        false);

                srcToBeGlobIo->getAsbFromWireMarker()->addUpdateMeta(connectEvent);
            }else{
                inputIo.connectTo(originOpr, false); /////// it may needs to be route
            }
            desSaveIo.push_back(&inputIo);
        }

    }




}

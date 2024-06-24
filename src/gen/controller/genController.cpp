//
// Created by tanawin on 20/6/2024.
//

#include "genController.h"

#include <model/controller/controller.h>


namespace kathryn{

    void GenController::initEnv(PARAM& param){
        _desVerilogPath = param[_desPathParamPrefix];
        _verilogWriter  = new FileWriterBase(_desVerilogPath);
        _masterModule   = getGlobalModulePtr();
    }

    void GenController::initEle(){
        _masterModule->createModuleGen();
        _masterModuleGen = _masterModule->getModuleGen();
        _masterModuleGen->startInitEle();
    }

    void GenController::routeIo(){
        _masterModuleGen->startRouteEle();
    }

    void GenController::generateEveryModule(){
        _masterModuleGen->startWriteFile(_verilogWriter);
    }

    void GenController::reset(){
        _verilogWriter->flush();
        delete _verilogWriter;
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


}

//
// Created by tanawin on 15/1/2025.
//

#include "paramValueGen.h"
#include "model/hwComponent/value/PmVal.h"

namespace kathryn{

    ParamValGen::ParamValGen(ModuleGen* mdGenMaster,
                             PmVal*     master):
    LogicGenBase(mdGenMaster,
                 (Assignable*) master,
                 (Identifiable*) master),
    _master(master){ assert(master != nullptr);}

    std::string ParamValGen::decParamVal() {
        return "parameter " + getOpr() + " = " + std::to_string(_master->_rawValue);
    }

    std::string ParamValGen::decOp(){
        return std::to_string(_master->getConstOpr());
    }

}
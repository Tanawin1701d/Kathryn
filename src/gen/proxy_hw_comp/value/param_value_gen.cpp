//
// Created by tanawin on 15/1/2025.
//

#include "param_value_gen.h"
#include "model/hw_component/value/pm_val.h"

namespace kathryn{

    ParamValGen::ParamValGen(ModuleGen* md_gen_master,
                             PmVal*     master):
    LogicGenBase(md_gen_master,
                 (Assignable*) master,
                 (Identifiable*) master),
    _master(master){ assert(master != nullptr);}

    std::string ParamValGen::dec_param_val() {
        return "parameter " + get_opr() + " = " + std::to_string(_master->_rawValue);
    }

    std::string ParamValGen::dec_op(){
        return std::to_string(_master->get_const_opr());
    }

}
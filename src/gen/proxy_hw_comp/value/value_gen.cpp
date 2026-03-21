//
// Created by tanawin on 20/6/2024.
//

#include "value_gen.h"
#include "model/hw_component/value/value.h"

namespace kathryn{

    ValueGen::ValueGen(ModuleGen*    md_gen_master,
                       Val*          master):
    LogicGenBase(md_gen_master,
                 (Assignable*) master,
                 (Identifiable*) master),

    _master(master){ assert(master != nullptr);}


    std::string ValueGen::get_val_str() const{

        if (_master->_rawValue == UINT64_MAX){
            return "-1";
        }

        return std::to_string(_master->_rawValue);
    }


    std::string ValueGen::dec_io(){return "";}



    std::string ValueGen::dec_variable(){
        Slice sl = _master->get_operable_slice();

        return "wire [" + std::to_string(sl.stop-1) +
            ": 0]" + get_opr() + " = " + get_val_str() + ";";
    }

    std::string ValueGen::dec_op(){return "";}

}
//
// Created by tanawin on 20/6/2024.
//

#include "reg_gen.h"
#include "model/hw_component/register/register.h"

namespace kathryn{

    RegGen::RegGen(ModuleGen* md_gen_master,
                   Reg* reg_master):
    AssignGenBase(
        md_gen_master,
        (Assignable*)reg_master,
        (Identifiable*)reg_master
    ),_master(reg_master){}

    std::string RegGen::dec_io(){
        assert(false);
    }

    std::string RegGen::dec_variable(){
        Slice sl = _master->get_operable_slice();
        //////////// it act as wire
        return "reg [" + std::to_string(sl.stop-1) +
                ": 0] " +get_opr() + ";";
    }

    std::string RegGen::dec_op(){
        return AssignGenBase::assign_op_base();
    }

    WIRE_MARKER_TYPE RegGen::get_glob_io_status(){
        _master->check_integrity();
        return _master->get_marker();
    }



}
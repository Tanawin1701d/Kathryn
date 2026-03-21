//
// Created by tanawin on 20/6/2024.
//

#ifndef REGGEN_H
#define REGGEN_H

#include "gen/proxy_hw_comp/abstract/assign_gen.h"
#include "model/hw_component/register/register.h"
#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"


namespace kathryn{

class Reg;
class RegGen: public AssignGenBase{
protected:
    Reg* _master = nullptr;
public:
    RegGen(ModuleGen*    md_gen_master,
            Reg*          reg_master);

    std::string dec_io()       override;
    std::string dec_variable() override;
    std::string dec_op()       override;

    WIRE_MARKER_TYPE get_glob_io_status() override;
};


}

#endif //REGGEN_H

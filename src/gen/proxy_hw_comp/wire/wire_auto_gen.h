//
// Created by tanawin on 20/6/2024.
//

#ifndef WIREAUTOGEN_H
#define WIREAUTOGEN_H
#include "model/hw_component/wire/wire_auto.h"
#include "gen/proxy_hw_comp/abstract/assign_gen.h"

namespace kathryn{

    class Wire;
    /** Wire which is NOT exposed to model it is auto generate wire
     * cause of auto generate is
     * auto input/output routing
     * global input/output wire representation
     */
    class WireAutoGen: public AssignGenBase{
    protected:
        WIRE_AUTO_GEN_TYPE _autoWireGenType; ///// active when is_model_wire is false
        WireAuto*          _master = nullptr;
    public:

        WireAutoGen(ModuleGen*    md_gen_master,
                    WireAuto*         wire_master,
                    WIRE_AUTO_GEN_TYPE  io_type);

        std::string get_opr()     override;

        std::string dec_io()       override;
        std::string dec_variable() override; ///// in case output there is no need dec variable
        std::string dec_op()       override;

    };

}

#endif //WIREAUTOGEN_H

//
// Created by tanawin on 20/6/2024.
//

#ifndef WIREGEN_H
#define WIREGEN_H

#include "gen/proxy_hw_comp/abstract/assign_gen.h"

namespace kathryn{

    class Wire;
    /**
     * Wire which is exposed to model it may be marked to be
     * input/output io wire or normal wire
     */
    class WireGen: public AssignGenBase{
    protected:
        Wire* _master = nullptr;
    public:

        WireGen(
            ModuleGen* md_gen_master,
            Wire*      wire_master
            );
        void route_dep() override;

        //std::string get_opr()     override;

        std::string dec_io()       override;
        std::string dec_variable() override; ///// in case output there is no need dec variable
        std::string dec_op()       override;

    };

}

#endif //WIREAUTOGEN_H

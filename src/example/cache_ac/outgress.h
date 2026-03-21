//
// Created by tanawin on 15/9/2024.
//

#ifndef OUTGRESS_H
#define OUTGRESS_H

#include "kathryn.h"
#include "interface.h"
#include "example/data_struct/field/dynamic_field.h"

namespace kathryn::cache_server{

    class OutgressBase: public Module{
    public:
        SERVER_PARAM&                _svParam;
        std::vector<BankOutputInterface*> _outputInterfaces;

        OutgressBase(SERVER_PARAM& sv_param,
                     std::vector<BankOutputInterface*> output_interfaces):
        _svParam(sv_param),
        _outputInterfaces(std::move(output_interfaces)){}

    };

}

#endif //OUTGRESS_H

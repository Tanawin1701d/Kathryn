//
// Created by tanawin on 15/9/2024.
//

#ifndef OUTGRESS_H
#define OUTGRESS_H

#include "kathryn.h"
#include "interface.h"
#include "example/dataStruct/field/dynamicField.h"

namespace kathryn::cacheServer{

    class OutgressBase: public Module{
    public:
        SERVER_PARAM                _svParam;
        std::vector<BankOutputInterface*> _outputInterfaces;

        OutgressBase(SERVER_PARAM svParam,
                     std::vector<BankOutputInterface*> outputInterfaces):
        _svParam(svParam),
        _outputInterfaces(std::move(outputInterfaces)){}

    };

}

#endif //OUTGRESS_H

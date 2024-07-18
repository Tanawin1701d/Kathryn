//
// Created by tanawin on 18/7/2024.
//

#ifndef VALUESIM_H
#define VALUESIM_H

#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"

namespace kathryn{

    class Val;
    class ValSimEngine: public LogicSimEngine{
    protected:
        Val* _master = nullptr;
    public:
        ValSimEngine(Val* master,VCD_SIG_TYPE sigType, ull rawValue);

        /////////// slice value
        std::string genSrcOpr                () override;
        std::string genSlicedOprTo           (Slice srcSlice) override;
        std::string genSlicedOprAndShift     (Slice desSlice, Slice srcSlice ) override;
        /////////// create global variable
        std::string createGlobalVariable() override;
        std::string createOp() override{return "";}
    };

}

#endif //VALUESIM_H

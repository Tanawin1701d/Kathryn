//
// Created by tanawin on 18/7/2024.
//

#ifndef VALUESIM_H
#define VALUESIM_H

#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"

namespace kathryn{

    constexpr char CXX_ULL_SUFFIX [] = "ULL";

    class Val;
    class ValSimEngine: public LogicSimEngine{
    protected:
        Val* _master = nullptr;
    public:
        ValSimEngine(Val* master,VCD_SIG_TYPE sigType, ull rawValue);

        /////////// slice value
        std::string genSrcOpr                () override;
        std::string genSlicedOprTo           (Slice srcSlice, SIM_VALREP_TYPE svt) override;
        std::string genSlicedOprAndShift     (Slice desSlice, Slice srcSlice,
                                              SIM_VALREP_TYPE svt ) override;
        /////////// create global variable
        void        createGlobalVariable(CbBaseCxx& cb) override;
        void        createOp(CbBaseCxx& cb) override{}
    };

}

#endif //VALUESIM_H

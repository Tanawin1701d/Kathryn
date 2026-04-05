//
// Created by tanawin on 14/1/2025.
//

#ifndef src_sim_modelSimEngine_hwComponent_value_PMVALUESIM_H
#define src_sim_modelSimEngine_hwComponent_value_PMVALUESIM_H
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"

namespace kathryn{

    class PmVal;
    class PmValSimEngine: public LogicSimEngine{
    protected:
        PmVal* _master = nullptr;
    public:
        PmValSimEngine(PmVal* master, VCD_SIG_TYPE sigType);

        ////////// to get the variable in generator
        ValR getValRep() override;
        ////////// to create variable
        void createGlobalVariable(CbBaseCxx& cb) override{}
        void createOp            (CbBaseCxx& cb) override{}




    };

}

#endif //src_sim_modelSimEngine_hwComponent_value_PMVALUESIM_H

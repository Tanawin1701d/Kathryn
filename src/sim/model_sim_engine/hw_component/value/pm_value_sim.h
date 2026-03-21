//
// Created by tanawin on 14/1/2025.
//

#ifndef src_sim_modelSimEngine_hwComponent_value_PMVALUESIM_H
#define src_sim_modelSimEngine_hwComponent_value_PMVALUESIM_H
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"

namespace kathryn{

    class PmVal;
    class PmValSimEngine: public LogicSimEngine{
    protected:
        PmVal* _master = nullptr;
    public:
        PmValSimEngine(PmVal* master, VCD_SIG_TYPE sig_type);

        ////////// to get the variable in generator
        ValR get_val_rep() override;
        ////////// to create variable
        void create_global_variable(CbBaseCxx& cb) override{}
        void create_op            (CbBaseCxx& cb) override{}




    };

}

#endif //src_sim_modelSimEngine_hwComponent_value_PMVALUESIM_H

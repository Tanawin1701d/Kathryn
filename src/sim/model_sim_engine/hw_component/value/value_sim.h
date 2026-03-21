//
// Created by tanawin on 18/7/2024.
//

#ifndef VALUESIM_H
#define VALUESIM_H

#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"

namespace kathryn{


    class Val;
    class ValSimEngine: public LogicSimEngine{
    protected:
        Val* _master = nullptr;
    public:
        ValSimEngine(Val* master,VCD_SIG_TYPE sig_type, ull raw_value);

        /////////// slice value
        ValR get_val_rep() override;
        /////////// create global variable
        void create_global_variable(CbBaseCxx& cb) override;
        void create_op(CbBaseCxx& cb) override{}
    };

}

#endif //VALUESIM_H

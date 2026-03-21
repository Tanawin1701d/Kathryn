//
// Created by tanawin on 18/7/2024.
//

#ifndef EXPRESSIONSIM_H
#define EXPRESSIONSIM_H
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"

namespace kathryn{

    class expression;
    class expression_sim_engine: public LogicSimEngine{
        expression* _master = nullptr;
        bool        _isCached = false;
    public:
        expression_sim_engine(expression* master, VCD_SIG_TYPE sig_type);
        void proxy_build_init() override;

        void create_global_variable(CbBaseCxx& cb) override;
        void create_local_variable (CbBaseCxx& cb)  override;
        void create_op            (CbBaseCxx& cb) override;
    };


}

#endif //EXPRESSIONSIM_H

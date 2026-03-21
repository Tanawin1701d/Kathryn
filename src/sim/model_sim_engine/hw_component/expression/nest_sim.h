//
// Created by tanawin on 18/7/2024.
//

#ifndef NESTSIM_H
#define NESTSIM_H
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"


namespace kathryn{

    class nest;
    class NestSimEngine: public LogicSimEngine{
        nest* _master = nullptr;
    public:
        NestSimEngine(nest* master, VCD_SIG_TYPE sig_type);
        void proxy_build_init() override;
        void create_op(CbBaseCxx& cb) override;

    };

}

#endif //NESTSIM_H

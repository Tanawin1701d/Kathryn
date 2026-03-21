//
// Created by tanawin on 18/7/2024.
//

#ifndef WIRESIM_H
#define WIRESIM_H

#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"
namespace kathryn{

    class Wire;
    class WireSimEngine: public LogicSimEngine{
        Wire* _master;
    public:
        WireSimEngine(Wire* master, VCD_SIG_TYPE sig_type);
        void mark_sv(const std::string& str) override {markSV_base(str);}
    };

}

#endif //WIRESIM_H

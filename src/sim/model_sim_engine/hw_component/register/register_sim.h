//
// Created by tanawin on 18/7/2024.
//

#ifndef REGISTERSIM_H
#define REGISTERSIM_H
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"

namespace kathryn{

    class Reg;
    class RegSimEngine: public LogicSimEngine{
        Reg* _master = nullptr;
    public:
        RegSimEngine(Reg* master,VCD_SIG_TYPE sig_type);
        void mark_sv(const std::string& str) override {markSV_base(str);}
    };

}

#endif //REGISTERSIM_H

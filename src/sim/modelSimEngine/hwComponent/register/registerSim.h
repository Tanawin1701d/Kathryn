//
// Created by tanawin on 18/7/2024.
//

#ifndef REGISTERSIM_H
#define REGISTERSIM_H
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"

namespace kathryn{

    class Reg;
    class RegSimEngine: public LogicSimEngine{
        Reg* _master = nullptr;
    public:
        RegSimEngine(Reg* master,VCD_SIG_TYPE sigType);
        void markSV(const std::string& str) override {markSV_base(str);}
    };

}

#endif //REGISTERSIM_H

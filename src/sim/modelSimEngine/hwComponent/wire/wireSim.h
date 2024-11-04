//
// Created by tanawin on 18/7/2024.
//

#ifndef WIRESIM_H
#define WIRESIM_H

#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
namespace kathryn{

    class Wire;
    class WireSimEngine: public LogicSimEngine{
        Wire* _master;
    public:
        WireSimEngine(Wire* master, VCD_SIG_TYPE sigType);
        void markSV(const std::string& str) override {markSV_base(str);}
    };

}

#endif //WIRESIM_H

//
// Created by tanawin on 18/7/2024.
//

#ifndef NESTSIM_H
#define NESTSIM_H
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"


namespace kathryn{

    class nest;
    class NestSimEngine: public LogicSimEngine{
        nest* _master = nullptr;
    public:
        NestSimEngine(nest* master, VCD_SIG_TYPE sigType);
        void proxyBuildInit() override;
        void createOp(CbBaseCxx& cb) override;

    };

}

#endif //NESTSIM_H

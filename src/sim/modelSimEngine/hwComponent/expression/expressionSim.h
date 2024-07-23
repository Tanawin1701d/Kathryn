//
// Created by tanawin on 18/7/2024.
//

#ifndef EXPRESSIONSIM_H
#define EXPRESSIONSIM_H
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"

namespace kathryn{

    class expression;
    class expressionSimEngine: public LogicSimEngine{
        expression* _master = nullptr;
        bool        _isCached = false;
    public:
        expressionSimEngine(expression* master, VCD_SIG_TYPE sigType);
        void proxyBuildInit() override;

        void createGlobalVariable(CbBaseCxx& cb) override;
        void createLocalVariable (CbBaseCxx& cb)  override;
        void createOp            (CbBaseCxx& cb) override;
    };


}

#endif //EXPRESSIONSIM_H

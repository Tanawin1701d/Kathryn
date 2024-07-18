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
        std::string _cachedPrecompile;
    public:
        expressionSimEngine(expression* master, VCD_SIG_TYPE sigType);
        void proxyBuildInit() override;
        /** override simulation engine */
        std::string getVarName() override;
        ///////

        std::string createPreCompile();
        std::string createGlobalVariable() override;
        std::string createLocalVariable()  override;
        std::string createOp() override;
    };


}

#endif //EXPRESSIONSIM_H

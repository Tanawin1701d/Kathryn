//
// Created by tanawin on 15/1/2025.
//

#ifndef KATHRYN_SRC_GEN_PROXYHWCOMP_VALUE_PARAMVALUEGEN_H
#define KATHRYN_SRC_GEN_PROXYHWCOMP_VALUE_PARAMVALUEGEN_H

#include "gen/proxyHwComp/abstract/logicGenBase.h"

namespace kathryn{

    class PmVal;
    class ParamValGen: public LogicGenBase{
        PmVal* _master = nullptr;

    public:
        explicit ParamValGen(ModuleGen* mdGenMaster,
                             PmVal*     master);

        void routeDep() override{};

        std::string decParamVal() override;

        std::string decOp() override;

    };

}

#endif //KATHRYN_SRC_GEN_PROXYHWCOMP_VALUE_PARAMVALUEGEN_H

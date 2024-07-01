//
// Created by tanawin on 20/6/2024.
//

#ifndef VALUEGEN_H
#define VALUEGEN_H
#include "gen/proxyHwComp/abstract/logicGenBase.h"
#include "model/hwComponent/value/value.h"

namespace kathryn{


    class Val;
    class ValueGen: public LogicGenBase{
        Val* _master = nullptr;

    public:

        explicit ValueGen(ModuleGen*        mdGenMaster,
                              Val*          master
        );

        void routeDep() override{};

        std::string decIo()       override;
        std::string decVariable() override;
        std::string decOp()       override;

        bool compare(LogicGenBase* lgb) override;

    };


}

#endif //VALUEGEN_H

//
// Created by tanawin on 20/6/2024.
//

#ifndef VALUEGEN_H
#define VALUEGEN_H
#include "gen/proxyHwComp/abstract/logicGenBase.h"
#include "model/hwComponent/value/value.h"

namespace kathryn{


    class ValueGen: public LogicGenBase{
        Val* _master = nullptr;

    public:

        explicit ValueGen(ModuleGen*        mdGenMaster,
                              logicLocalCef cerf,
                              Val*          master
        );

        void routeDep() override{};

        std::string getOpr()         override;
        std::string getOpr(Slice sl) override;

        std::string decIo()       override;
        std::string decVariable() override;
        std::string decOp()       override;

    };


}

#endif //VALUEGEN_H

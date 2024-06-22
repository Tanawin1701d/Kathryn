//
// Created by tanawin on 20/6/2024.
//

#ifndef REGGEN_H
#define REGGEN_H

#include "gen/proxyHwComp/abstract/AssignGen.h"
#include "model/hwComponent/register/register.h"
#include "gen/proxyHwComp/abstract/logicGenBase.h"


namespace kathryn{


class RegGen: public AssignGenBase{
protected:
    Reg* _master = nullptr;
public:
    RegGen(ModuleGen*    mdGenMaster,
            logicLocalCef cerf,
            Assignable*   asb,
            Reg*          regMaster);

    std::string getOpr() override;
    std::string getOpr(Slice sl) override;

    std::string decIo()       override;
    std::string decVariable() override;
    std::string decOp()       override;
};


}

#endif //REGGEN_H

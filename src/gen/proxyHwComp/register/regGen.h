//
// Created by tanawin on 20/6/2024.
//

#ifndef REGGEN_H
#define REGGEN_H

#include "gen/proxyHwComp/abstract/AssignGen.h"
#include "model/hwComponent/register/register.h"
#include "gen/proxyHwComp/abstract/logicGenBase.h"


namespace kathryn{

class Reg;
class RegGen: public AssignGenBase{
protected:
    Reg* _master = nullptr;
public:
    RegGen(ModuleGen*    mdGenMaster,
            Reg*          regMaster);

    std::string decIo()       override;
    std::string decVariable() override;
    std::string decOp()       override;

    bool compare(LogicGenBase* lgb) override;

    WIRE_MARKER_TYPE getGlobIoStatus() override;
};


}

#endif //REGGEN_H

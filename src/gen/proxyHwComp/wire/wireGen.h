//
// Created by tanawin on 20/6/2024.
//

#ifndef WIREGEN_H
#define WIREGEN_H
#include "gen/proxyHwComp/abstract/AssignGen.h"
#include "model/hwComponent/wire/wireSubType.h"

namespace kathryn{

    class Wire;
    class WireGen: public AssignGenBase{
    protected:
        WIRE_IO_TYPE _ioType;
        Wire* _master = nullptr;
    public:
        WireGen(ModuleGen*    mdGenMaster,
                logicLocalCef cerf,
                Wire*         wireMaster);

        WireGen(ModuleGen*    mdGenMaster,
                logicLocalCef cerf,
                Wire*         wireMaster,
                WIRE_IO_TYPE  ioType);

        std::string getOpr()     override;

        std::string decIo()       override;
        std::string decVariable() override; ///// in case output there is no need dec variable
        std::string decOp()       override;

        GLOB_IO_TYPE getGlobIoStatus() override;


    };

}

#endif //WIREGEN_H

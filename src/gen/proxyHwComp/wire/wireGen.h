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
        bool _isWireIo = false;
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

        std::string decIo()       override;
        std::string decVariable() override;
        std::string decOp()       override;
    };

}

#endif //WIREGEN_H

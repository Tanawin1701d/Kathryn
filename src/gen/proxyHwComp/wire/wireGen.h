//
// Created by tanawin on 20/6/2024.
//

#ifndef WIREGEN_H
#define WIREGEN_H
#include "model/hwComponent/wire/wire.h"
#include "gen/proxyHwComp/abstract/AssignGen.h"

namespace kathryn{

    class WireGen: public AssignGenBase{
    protected:
        bool  isIoWire = false;
        bool  isInput  = false; /// else is output
        Wire* _master = nullptr;
    public:
        WireGen(ModuleGen*    mdGenMaster,
                logicLocalCef cerf,
                Wire*         wireMaster);

        std::string getOpr() override;
        std::string getOpr(Slice sl) override;

        std::string decIo()       override;
        std::string decVariable() override;
        std::string decOp()       override;
    };

}

#endif //WIREGEN_H

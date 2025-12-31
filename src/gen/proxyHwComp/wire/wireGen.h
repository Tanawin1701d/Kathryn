//
// Created by tanawin on 20/6/2024.
//

#ifndef WIREGEN_H
#define WIREGEN_H

#include "gen/proxyHwComp/abstract/AssignGen.h"

namespace kathryn{

    class Wire;
    /**
     * Wire which is exposed to model it may be marked to be
     * input/output io wire or normal wire
     */
    class WireGen: public AssignGenBase{
    protected:
        Wire* _master = nullptr;
    public:

        WireGen(
            ModuleGen* mdGenMaster,
            Wire*      wireMaster
            );
        void routeDep() override;

        //std::string getOpr()     override;

        std::string decIo()       override;
        std::string decVariable() override; ///// in case output there is no need dec variable
        std::string decOp()       override;

    };

}

#endif //WIREAUTOGEN_H

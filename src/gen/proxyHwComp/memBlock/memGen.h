//
// Created by tanawin on 20/6/2024.
//

#ifndef MEMGEN_H
#define MEMGEN_H
#include "model/hwComponent/memBlock/MemBlock.h"
#include "gen/proxyHwComp/abstract/logicGenBase.h"


namespace kathryn{



    class MemGen: public LogicGenBase{

        MemBlock* _master;

    public:
        explicit MemGen(ModuleGen* mdGenMaster,
                        logicLocalCef cerf,
                        MemBlock* memBlockMaster);

    };

    class MemEleholderGen: public LogicGenBase{

    };


}

#endif //MEMGEN_H

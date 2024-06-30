//
// Created by tanawin on 20/6/2024.
//

#ifndef MEMGEN_H
#define MEMGEN_H
#include "model/hwComponent/memBlock/MemBlock.h"
#include "gen/proxyHwComp/abstract/logicGenBase.h"


namespace kathryn{



    class MemGen: public LogicGenBase{

        MemBlock* _master = nullptr;

    public:
        explicit MemGen(ModuleGen* mdGenMaster,
                        MemBlock* memBlockMaster);

        void routeDep() override{};

        std::string decIo      () override {return "";}
        std::string decVariable() override;
        std::string decOp      () override {return "";}

    };
    //



}

#endif //MEMGEN_H

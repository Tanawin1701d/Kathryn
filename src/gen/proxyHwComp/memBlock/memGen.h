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

        /////// route dep
        void routeDep() override{};
        ////// genCerf
        void genCerf(MODULE_GEN_GRP mgg, int grpIdx, int idx) override;

        std::string decIo      () override {return "";}
        std::string decVariable() override;
        std::string decOp      () override {return "";}

        bool compare(LogicGenBase* lgb) override;

        std::string getOpr(Slice sl) override;

    };
    //



}

#endif //MEMGEN_H

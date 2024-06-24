//
// Created by tanawin on 23/6/2024.
//

#ifndef MEMAGENTGEN_H
#define MEMAGENTGEN_H

#include"gen/proxyHwComp/abstract/AssignGen.h"



namespace kathryn{

    class MemEleholderGen: public AssignGenBase{
    protected:
        MemBlockEleHolder* _master     = nullptr;
        Operable*          routedIndexer = nullptr;
    public:
        explicit MemEleholderGen(
            ModuleGen*  mdGenMaster,
            logicLocalCef cerf,
            MemBlockEleHolder* mhMaster
        );

        void routeDep() override;

        std::string decIo() override {assert(false);}
        std::string decVariable() override;
        std::string decOp() override;

        std::string assignmentLine(Slice desSlice, Operable* srcUpdateValue) override;
    };

}

#endif //MEMAGENTGEN_H

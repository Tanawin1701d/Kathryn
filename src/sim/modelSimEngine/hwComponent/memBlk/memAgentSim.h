//
// Created by tanawin on 18/7/2024.
//

#ifndef MEMAGENTSIM_H
#define MEMAGENTSIM_H

#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"

namespace kathryn{


    class MemBlockEleHolder;
    class MemEleHolderSimEngine: public LogicSimEngine{
        const std::string IS_SET_SUFFIX = "_isSet";
        const std::string INDEXER_SUFFIX = "_indexer";
        MemBlockEleHolder* _master = nullptr;
    public:
        MemEleHolderSimEngine(MemBlockEleHolder* master);

        void        proxyBuildInit()    override;

        std::string createGlobalVariable() override;
        std::string createLocalVariable () override;

        std::string createOp()             override;
        std::string createOpEndCycle()     override;
        std::string createOpEndCycle2()    override{return "";};

        std::string getIsSetVar();
        std::string getIndexerVar();

        std::string createOpReadMode();
        std::string createOpWriteMode();

    };

}

#endif //MEMAGENTSIM_H

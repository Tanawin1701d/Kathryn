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
        explicit MemEleHolderSimEngine(MemBlockEleHolder* master);

        void        proxyBuildInit()    override;

        void createGlobalVariable(CbBaseCxx& cb) override;
        void createLocalVariable (CbBaseCxx& cb) override;

        void createOp         (CbBaseCxx& cb) override;
        void createOpEndCycle (CbBaseCxx& cb) override;
        void createOpEndCycle2(CbBaseCxx& cb) override{}

        std::string getIsSetVar();
        std::string getIndexerVar();

        void createOpReadMode (CbBaseCxx& cb);
        void createOpWriteMode(CbBaseCxx& cb);

    };

}

#endif //MEMAGENTSIM_H

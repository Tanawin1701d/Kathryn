//
// Created by tanawin on 23/6/2024.
//

#include "memGen.h"
#include "memAgentGen.h"
#include "gen/proxyHwComp/module/moduleGen.h"

namespace kathryn{

    MemEleholderGen::MemEleholderGen(
        ModuleGen*  mdGenMaster,
        logicLocalCef cerf,
        MemBlockEleHolder* mhMaster
    ): AssignGenBase(
        mdGenMaster,
        cerf,
        (Assignable*) mhMaster,
        (Identifiable*) mhMaster
    ),
    _master(mhMaster){}


    void MemEleholderGen::routeDep(){
        //////////// route from indexer
        routedIndexer =
            _mdGenMaster->routeSrcOprToThisModule(_master->getIndexer());
        //////////// route all from assignable
        AssignGenBase::routeDep();
    }

    std::string MemEleholderGen::decVariable(){

        if (_master->isWriteMode()){
            return "";
        }

        Slice sl = _master->getOperableSlice();
        ///////// read mode
        return "wire [" + std::to_string(sl.stop-1) +
            ": 0] " + getOpr() + ";";

    }

    std::string MemEleholderGen::decOp(){

        std::string   masterMemBlkOpr = _master->getMasterMemBlk()
                                        ->getLogicGen()->getOpr();
        std::string   indexerOpr      = getOprStrFromOpr(routedIndexer);

        if (_master->isWriteMode()){
            return assignOpBase(true);
        }
        //////////// readMode
        return "assign " + getOpr() + " = " +
            masterMemBlkOpr + "[" + indexerOpr + "];";

    }


    std::string MemEleholderGen::assignmentLine(
        Slice desSlice,
        Operable* srcUpdateValue
    ){
        /////////// it only used in write
        assert(desSlice == _master->getOperableSlice());
        assert(srcUpdateValue != nullptr);

        std::string   masterMemBlkOpr = _master->getMasterMemBlk()
                                        ->getLogicGen()->getOpr();
        std::string   indexerOpr      = getOprStrFromOpr(routedIndexer);

        return masterMemBlkOpr + "[" + indexerOpr + "] <= " +
            getOprStrFromOpr(srcUpdateValue) + ";";
    }

}
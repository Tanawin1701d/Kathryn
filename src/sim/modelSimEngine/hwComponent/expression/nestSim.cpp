//
// Created by tanawin on 18/7/2024.
//

#include "nestSim.h"
#include "model/hwComponent/expression/nest.h"

namespace kathryn{


    /***
     *
     * NestLogicSim
     *
     * */

    NestSimEngine::NestSimEngine(nest* master,
                             VCD_SIG_TYPE sigType):
            LogicSimEngine(master, master, sigType, false, 0),
            _master(master){
        assert(_master != nullptr);
    }

    void NestSimEngine::proxyBuildInit(){
        for (NestMeta& meta: _master->_nestList){
            dep.push_back(meta.opr->getLogicSimEngineFromOpr());
        }
    }


    void NestSimEngine::createOp(CbBaseCxx& cb){
        ///////// build string
        cb.addCm(_ident->getGlobalName());

        /////////// we build from low priority to high priority
        int startIdx = 0;

        for (NestMeta& meta: _master->_nestList){
            //////// data preparation
            Operable* opr   = meta.opr;
            Assignable* asb = meta.asb;
            assert(opr != nullptr);
            if (!_master->readOnly){
                assert(asb != nullptr);
            }
            int curSize = opr->getOperableSlice().getSize();
            cb.addSt(genAssignAEqB({startIdx, startIdx + curSize}, false, opr));
            startIdx += curSize;
        }
        assert(startIdx == _asb->getAssignSlice().getSize());
    }

}
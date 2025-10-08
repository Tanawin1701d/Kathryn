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
            LogicSimEngine(master, master, master, sigType, false, 0),
            _master(master){
        assert(_master != nullptr);
    }

    void NestSimEngine::proxyBuildInit(){
        for (NestMeta& meta: _master->_nestList){
            dep.push_back(meta.opr1->getLogicSimEngineFromOpr());
        }
    }


    void NestSimEngine::createOp(CbBaseCxx& cb){
        ///////// build string
        cb.addCm(_ident->getGlobalName());

        /////////// we build from low priority to high priority
        int startIdx = 0;

        for (NestMeta& meta: _master->_nestList){
            //////// data preparation
            Operable* opr1   = meta.opr1;
            Assignable* asb = meta.asb;
            assert(opr1 != nullptr);
            if (!_master->readOnly){
                assert(asb != nullptr);
            }
            int curSize = opr1->getOperableSlice().getSize();
            cb.addSt(genAssignAEqB({startIdx, startIdx + curSize}, false, opr1));
            startIdx += curSize;
        }
        assert(startIdx == _asb->getAssignSlice().getSize());
    }

}
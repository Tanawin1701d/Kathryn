//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_REGMGM_H
#define KATHRYN_REGMGM_H

#include"storageMgm.h"

namespace kathryn{


    class RegMgmt: public StorageMgmt{

    public:

        makeReg(ReaderRegIdx      , IDX_SIZE);
        makeReg(ExecuteSecRegIdx  , IDX_SIZE);
        makeReg(WriteBackSecRegIdx, IDX_SIZE);

        explicit RegMgmt(int idxSize, int regSize):
                StorageMgmt(idxSize, regSize){}

        Operable& isOverlapWithWriteReg(Operable& idx){
            return (idx != 0) && ( (idx == ReaderRegIdx) || (idx == ExecuteSecRegIdx) || (idx == WriteBackSecRegIdx));

        }

        void stepFromReadMem(Reg& nextToWriteOpIdx){
            assert(nextToWriteOpIdx.getSlice().getSize() == IDX_SIZE);
            WriteBackSecRegIdx <<= ExecuteSecRegIdx;
            ExecuteSecRegIdx   <<= ReaderRegIdx;
            ReaderRegIdx       <<= nextToWriteOpIdx;
        }

        void reqExecuteFlushOccur(){
            ReaderRegIdx <<= 0;
        }


    };


}

#endif //KATHRYN_REGMGM_H

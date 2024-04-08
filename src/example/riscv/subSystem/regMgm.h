//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_REGMGM_H
#define KATHRYN_REGMGM_H

#include"storageMgm.h"
#include "example/riscv/element.h"

namespace kathryn{

    namespace riscv {


        class RegMgmt : public StorageMgmt {

        public:

            makeReg(ReaderRegIdx, IDX_SIZE);
            makeReg(ExecuteSecRegIdx, IDX_SIZE);
            makeReg(WriteBackSecRegIdx, IDX_SIZE);

            explicit RegMgmt(int idxSize, int regSize) :
                    StorageMgmt(idxSize, regSize) {}

            Operable &isOverlapWithWriteReg(RegEle& regEle) {
                return (regEle.idx != 0) &&
                       ((regEle.idx == ReaderRegIdx) ||
                       (regEle.idx == ExecuteSecRegIdx) ||
                       (regEle.idx == WriteBackSecRegIdx));

            }

            void stepFromReadMem(Reg &nextToWriteOpIdx) {
                assert(nextToWriteOpIdx.getSlice().getSize() == IDX_SIZE);
                WriteBackSecRegIdx <<= ExecuteSecRegIdx;
                ExecuteSecRegIdx <<= ReaderRegIdx;
                ReaderRegIdx <<= nextToWriteOpIdx;
            }

            void reqExecuteFlushOccur() {
                ReaderRegIdx <<= 0;
            }


            void reqReg(RegEle& regEle){
                reqStorage(regEle.val, regEle.idx, regEle.valid);
            }



        };

    }


}

#endif //KATHRYN_REGMGM_H

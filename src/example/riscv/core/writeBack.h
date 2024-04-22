//
// Created by tanawin on 8/4/2567.
//

#ifndef KATHRYN_WRITEBACK_H
#define KATHRYN_WRITEBACK_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "example/riscv/subSystem/storageMgm.h"

namespace kathryn{

    namespace riscv{

        class WriteBack{
        public:

            void flow(UOp& execUop, MemBlock& regFile, BYPASS_DATA& bypassData){
                RegEle& desReg = execUop.regData[RS_des];
                pipBlk{
                    zif( (desReg.valid) && (desReg.idx != 0)){
                        regFile[desReg.idx] <<= desReg.val;
                        bypassData.idx   = desReg.idx;
                        bypassData.value = desReg.val;

                    }
                }
            }
        };
    }
}

#endif //KATHRYN_WRITEBACK_H

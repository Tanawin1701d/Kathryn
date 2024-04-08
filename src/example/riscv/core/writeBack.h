//
// Created by tanawin on 8/4/2567.
//

#ifndef KATHRYN_WRITEBACK_H
#define KATHRYN_WRITEBACK_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "execute.h"

namespace kathryn{

    namespace riscv{

        class WriteBack{
        public:
            makeReg(x, 1);
            void flow(Execute& exec,RegMgmt& regMgmt){
                RegEle& nextToWriteReg = exec.exUop.regData[RS_des];
                pipBlk{
                    cif(nextToWriteReg.valid) {
                        regMgmt.reqWriteReg(nextToWriteReg.val, nextToWriteReg.idx);
                    }celse{
                        x <<= 0;
                    }
                }
            }

        };

    }

}

#endif //KATHRYN_WRITEBACK_H

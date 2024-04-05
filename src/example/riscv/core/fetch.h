//
// Created by tanawin on 3/4/2567.
//

#ifndef KATHRYN_FETCH_H
#define KATHRYN_FETCH_H

#include "kathryn.h"
#include "example/riscv/subSystem/regMgm.h"
#include "example/riscv/parameter.h"

namespace kathryn{

    namespace riscv {

        class Fetch {


        public:
            makeReg(fetch_instr, XLEN);
            makeReg(fetch_pc, MEM_ADDR_IDX);
            makeReg(fetch_nextpc, MEM_ADDR_IDX);

            void flow(StorageMgmt &memMgmt, Operable &reqPc) {
                pipBlk {
                    par {
                        memMgmt.reqReadReg(fetch_instr, reqPc);
                        fetch_pc <<= reqPc;
                    }
                }
            }

        };

    }

}

#endif //KATHRYN_FETCH_H

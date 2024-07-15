//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_ELEMENT_H
#define KATHRYN_ELEMENT_H


#include "kathryn.h"
#include "parameter.h"
#include "lib/instr/instrBase.h"


namespace kathryn{

    namespace riscv{

        struct UOp{
            InstrRepo repo;
            mReg(pc, XLEN);
            mReg(nextPc, XLEN);

            explicit UOp(Operable* instr):
                        repo(XLEN, AMT_DEC_SRC_REG,
                        AMT_DEC_DES_REG ,XLEN,instr)
            {assert(instr != nullptr);}
        };

        struct FETCH_DATA{
            mReg(fetch_pc    , MEM_ADDR_IDX);
            mReg(fetch_nextpc, MEM_ADDR_IDX);
            mReg(fetch_instr, XLEN);
        };

        struct BYPASS_DATA{
            mWire(idx, REG_IDX);
            mWire(value, XLEN);
        };

    }

}

#endif //KATHRYN_ELEMENT_H

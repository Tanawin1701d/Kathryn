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

        struct FETCH_DATA{
            SyncMeta sync{"fetchSync"};
            mReg(fetch_pc    , MEM_ADDR_IDX);
            mReg(fetch_nextpc, MEM_ADDR_IDX);
            mReg(fetch_instr, XLEN);
        };

        struct DECODE_DATA{
            InstrRepo repo;
            mReg(pc, XLEN);
            mReg(nextPc, XLEN);
            SyncMeta sync{"decodeSync"};

            explicit DECODE_DATA(Operable& instr):
                repo(XLEN, AMT_DEC_SRC_REG,
                AMT_DEC_DES_REG ,XLEN,&instr){}
        };

        struct EXEC_DATA{
            SyncMeta sync{"execSync"};
            OPR_HW   wbData{XLEN, REG_IDX,0, false}; //// write back data
        };

        struct WRITE_BACK_DATA{

            SyncMeta sync{"wbSync"};
        };

        struct BYPASS_DATA{
            mWire(idx, REG_IDX);
            mWire(value, XLEN);
        };

    }

}

#endif //KATHRYN_ELEMENT_H

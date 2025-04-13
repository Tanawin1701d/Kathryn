//
// Created by tanawin on 11/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_EXEC_BRUNIT_H
#define KATHRYN_SRC_KRIDE_EXEC_BRUNIT_H

#include "kride/krideParam.h"
#include "EUBase.h"
#include "alu.h"

namespace kathryn{

    struct BrUnit: EUBase{



        BrUnit(D_ALL& din, D_IO_RSV& dcIn):
        EUBase(din, dcIn){}

        void flow() override{
            Slot&      sl = d.rsvBranch.issueBuf;

            //////// no src to select due to src of operation is the from
            /// register only
            auto& bcmp = ALU().exec(sl.at("aluOp"),
                                   sl.at("src1"),
                                   sl.at("src2"));

            ////////// branch condition
            mWire(curBrCond, 1);
            auto& opcode = sl.at("opcode");
            auto& mustBrCond = (opcode == RV32_JAL) || (opcode == RV32_JALR);
            curBrCond = mustBrCond || ((~mustBrCond) && bcmp);
            /////////// jump src
            mWire(jumpBsAddr, ADDR_LEN); ///// bs = base
            zif(opcode == RV32_JALR) jumpBsAddr = sl.at("src1");
            zelse                    jumpBsAddr = sl.at("pc");
            ///////// jump res
            mWire(jumpRsAddr, ADDR_LEN); ///// rs = result
            zif(curBrCond)  jumpRsAddr = jumpBsAddr + sl.at("imm");
            zelse           jumpRsAddr = sl.at("pc") + 4;
            ////////////////////////////////////////////////////////////////////
            /// pip line timing
            pip(EX_BR){
                ////// no interrupt because it is the only cause of interrupt
                ////////////// TODO pipe wake
                writeReg(sl, sl.at("pc") + 4);
                ////////////// do predict Raise
                curBrCond != sl.at("prCond");
                ///////TODO send req to offer who have this
            }
        }
    };

}

#endif //KATHRYN_SRC_KRIDE_EXEC_BRUNIT_H

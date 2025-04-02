//
// Created by tanawin on 25/3/2025.
//

#ifndef SRC_KRIDE_DATATYPE_H
#define SRC_KRIDE_DATATYPE_H

#include "kathryn.h"
#include "lib/hw/slot/slot.h"
#include "krideParam.h"

namespace kathryn{

    struct D_EXEC_BRANCH{
        mWire(sucPred      , 1);
        mWire(misPred      , 1);
        mWire(rrfIdxRcv    , RRF_SEL);
        mWire(specTagRcvIdx, SPEC_TAG_SEL);
        mWire(specTagRcv   , SPEC_TAG_LEN);
    };


    struct D_COMMIT{
        mWire(comAmt        , RRF_SEL+1);
        mWire(nxtCommitIdx  , RRF_SEL+1);

        mWire(commitAddr0   , ARF_SEL);
        mWire(commitAddr1   , ARF_SEL);


    };

    struct D_DISPATCH{
        mWire(rmAmt, RRF_SEL+1);
        mWire(specTagPoolReq, SPEC_TAG_LEN);

    };

    struct D_INSTR{
        mReg(instr0, INSTR_LEN);
        mReg(instr1, INSTR_LEN);
    };

    struct D_DECODE{

        RegSlot dec;

        D_DECODE(): dec({"immType"     , "rs1Idx"        , "rs2Idx" , "rdIdx",  ///// operand section
                                         "srcASel"       , "srcBSel", "regWr",
                                         "useRs1"        , "useRs2",
                         "illInstr"    , "aluOp"         , "rsEntry",            ////// operation section
                         "dmemSize"    , "dmemType"      ,
                         "mdOp"        , "isMd1Sig"      , "isMd2Sig", "mdOutSel"},
                        {IMM_TYPE_WIDTH, ARF_SEL, ARF_SEL, ARF_SEL,
                                         SRC_A_SEL_WIDTH, SRC_A_SEL_WIDTH, 1,
                                         1              , 1,
                         1             , ALU_OP_WIDTH   , RS_ENT_SEL,
                         MEM_TYPE_WIDTH, MEM_TYPE_WIDTH ,
                         MD_OP_WIDTH   , MD_OUT_SEL_WIDTH
                        }){}

        void decode(Reg& instr); //// TODO decode the fucking instruction
    };


    struct D_ALL{
        D_EXEC_BRANCH exb;
        D_COMMIT      com;
        D_DISPATCH    dis;
    };




}

#endif //SRC_KRIDE_DATATYPE_H

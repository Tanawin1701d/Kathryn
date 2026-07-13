//
// Created by tanawin on 23/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

#include "parameter.h"

using namespace std;

namespace kathryn::o3{

        /**
         *   FETCH STAGE
         */

        inline SlotMeta smFetch {                                 ///CTRL_HWD+DATA_HWD SHARED_COMP
            {inst1   , inst2   , invalid2,       pc,      npc},   ///PARAM SHARED_COMP
            {INSN_LEN, INSN_LEN, 1       , ADDR_LEN, ADDR_LEN}    ///PARAM SHARED_COMP
        };

        ///inline SlotMeta smBhrs{bhr, GSH_BHR_LEN, SPECTAG_LEN, 0};

        /**
         * DECODE STAGE
         */

        inline SlotMeta smDecBase {                                                                            ///CTRL_HWD+DATA_HWD SHARED_COMP
            {inst              , invalid           , imm_type    , aluOp          , rsEnt, /////// operation   ///PARAM SHARED_COMP
             isBranch          , pred_addr         , spec        , specTag        , illLegal, ////// branch    ///PARAM SHARED_COMP
             rdIdx             , rdUse             ,    /////// destination ///PARAM SHARED_COMP
             rsIdx_1           , rsSel_1           , rsUse_1     , /////// src1 ///PARAM SHARED_COMP
             rsIdx_2           , rsSel_2           , rsUse_2     , /////// src2 ///PARAM SHARED_COMP
             dmem_size         , dmem_type         , //////// memory ///PARAM SHARED_COMP
             md_req_op         , md_req_in_signed_1, //////// multiplication ///PARAM SHARED_COMP
             md_req_in_signed_2, md_req_out_sel ///PARAM SHARED_COMP
            },

            {INSN_LEN          , 1                 , IMM_TYPE_WIDTH , ALU_OP_WIDTH   , RS_ENT_SEL,   ///PARAM SHARED_COMP
             1                 , ADDR_LEN          , 1              , SPECTAG_LEN    , 1         , ///PARAM SHARED_COMP
             REG_SEL           , 1                 , ///PARAM SHARED_COMP
             REG_SEL           , SRC_A_SEL_WIDTH   , 1              , ///PARAM SHARED_COMP
             REG_SEL           , SRC_B_SEL_WIDTH   , 1              , ///PARAM SHARED_COMP
             3                 , MEM_TYPE_WIDTH    , ///PARAM SHARED_COMP
             MD_OP_WIDTH       , 1                 , ///PARAM SHARED_COMP
             1                 , MD_OUT_SEL_WIDTH ///PARAM SHARED_COMP
            }
        };

        inline SlotMeta smDecShard {   ///CTRL_HWD+DATA_HWD SHARED_COMP
            {      pc, desEqSrc1, desEqSrc2},   ///PARAM SHARED_COMP
            {ADDR_LEN, 1        , 1        }   ///PARAM SHARED_COMP
        };

        /**
         * RSV
         */
        inline SlotMeta smRsvBase {   ///CTRL_HWD+DATA_HWD SHARED_COMP
            {pc         , rrftag          , rdUse     , aluOp,   ///PARAM SHARED_COMP
             spec       , specTag         , ///PARAM SHARED_COMP
             phyIdx_1   , rsSel_1         , rsValid_1 , ///PARAM SHARED_COMP
             phyIdx_2   , rsSel_2         , rsValid_2 ///PARAM SHARED_COMP
            },
            ////////////////////////////////////////////////////////////////////
            {ADDR_LEN   , RRF_SEL         , 1          , ALU_OP_WIDTH,   ///PARAM SHARED_COMP
             1          , SPECTAG_LEN     , ///PARAM SHARED_COMP
             DATA_LEN   , SRC_A_SEL_WIDTH , 1          , ///PARAM SHARED_COMP
             DATA_LEN   , SRC_B_SEL_WIDTH , 1 ///PARAM SHARED_COMP
            }
        };

        inline SlotMeta smRsvBranch{   ///DATA_HWD SHARED_COMP
            {opcode      , pred_addr, imm_br},   ///PARAM SHARED_COMP
            {OPCODE_WIDTH, ADDR_LEN , ADDR_LEN}   ///PARAM SHARED_COMP
        };

        inline SlotMeta smRsvAlu{   ///DATA_HWD SHARED_COMP
            {imm},   ///PARAM SHARED_COMP
            {DATA_LEN}   ///PARAM SHARED_COMP
        };

        inline SlotMeta smRsvMul {   ///DATA_HWD SHARED_COMP
            {md_req_in_signed_1, md_req_in_signed_2, md_req_out_sel   },   ///PARAM SHARED_COMP
            {1                 , 1                 , MD_OUT_SEL_WIDTH }   ///PARAM SHARED_COMP
        };

        inline SlotMeta smRsvO{ /// out of order   ///CTRL_HWD SHARED_COMP
            {busy, sortBit},                       ///PARAM SHARED_COMP
            {1   , 1      }   ///PARAM SHARED_COMP
        };

        inline SlotMeta smRsvI{   ///CTRL_HWD SHARED_COMP
                {busy},   ///PARAM SHARED_COMP
                {1}   ///PARAM SHARED_COMP
        };

        inline SlotMeta smBtb{   ///DC
                {busy},   ///DC
                {64}   ///DC
        };

        inline SlotMeta smStoreBuf{ //// for store buffer entry   ///CTRL_HWD+DATA_HWD SHARED_COMP
                    { busy    ,  complete, spec     , specTag    ,   ///PARAM SHARED_COMP
                      mem_addr ///PARAM SHARED_COMP
                    },
                    { 1       , 1        , 1        , SPECTAG_LEN,   ///PARAM SHARED_COMP
                      ADDR_LEN ///PARAM SHARED_COMP
                    }
        };

        inline SlotMeta smLdSt{  //// for second stage system   ///CTRL_HWD+DATA_HWD SHARED_COMP
                    { rrftag   , rdUse   , spec, specTag,   ///PARAM SHARED_COMP
                      stBufData, stBufHit ///PARAM SHARED_COMP
                    },
                    { RRF_SEL  , 1       , 1   , SPECTAG_LEN,   ///PARAM SHARED_COMP
                      DATA_LEN , 1 ///PARAM SHARED_COMP
                    }
        };

    /**
     * MPFT
     */
    inline SlotMeta smMPFT{   ///CTRL_HWD SHARED_COMP
        {mpft_valid, mpft_fixTag},   ///PARAM SHARED_COMP
        {1         , SPECTAG_LEN}   ///PARAM SHARED_COMP
    };

    /**
     * RRF
     */
    inline SlotMeta smRRF{   ///CTRL_HWD+DATA_HWD SHARED_COMP
        {rrfValid, rrfData },   ///PARAM SHARED_COMP
        {1       , DATA_LEN}   ///PARAM SHARED_COMP
    };

    /**
     *  ARF
     */
    ////////// | arfBusy_0 | arfBusy_1 | arfBusy_2 ..... | arfBusy_31
    inline SlotMeta smARFBusy{arfBusy, 1, REG_NUM, 0};   ///CTRL_HWD SHARED_COMP
    ////////// | arfRenamed_0 | arfRenamed_1 | arfRenamed_2 ..... | arfRenamed_31
    inline SlotMeta smARFRenamed{arfRenamed, RRF_SEL, REG_NUM, 0};   ///CTRL_HWD SHARED_COMP
    ////////// | rrfData_0 | rrfData_1 | rrfData_2 ..... | rrfData_31
    inline SlotMeta smARFData{arfData, DATA_LEN, REG_NUM, 0};   ///DATA_HWD SHARED_COMP

    /**
     * ROB
     */
    inline SlotMeta smROB{    ///// check it   ///CTRL_HWD+DATA_HWD SHARED_COMP
        {wbFin, isBranch, storeBit,   ///PARAM SHARED_COMP
         rdUse, rdIdx   , pc       }, ///PARAM SHARED_COMP
        /////////////////////////////////////////////////
        {1    , 1       , 1       ,   ///PARAM SHARED_COMP
         1    , REG_SEL , ADDR_LEN} ///PARAM SHARED_COMP
    };

}


#endif //KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

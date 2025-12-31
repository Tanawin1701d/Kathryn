//
// Created by tanawin on 23/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

#include "kathryn.h"
#include "parameter.h"

using namespace std;

namespace kathryn::o3{

        /**
         *   FETCH STAGE
         */

        inline SlotMeta smFetch {
            {inst1   , inst2   , invalid2,       pc,      npc, prCond, bhr        },
            {INSN_LEN, INSN_LEN, 1       , ADDR_LEN, ADDR_LEN, 1     , GSH_BHR_LEN}
        };

        inline SlotMeta smBhrs{bhr, GSH_BHR_LEN, SPECTAG_LEN, 0};

        /**
         * DECODE STAGE
         */

        inline SlotMeta smDecBase {
            {inst             , invalid           , imm_type    , aluOp          , rsEnt, /////// operation
             isBranch         , pred_addr         , spec        , specTag        , illLegal, ////// branch
             rdIdx            , rdUse             ,    /////// destination
             rsIdx_1          , rsSel_1           , rsUse_1     , /////// src1
             rsIdx_2          , rsSel_2           , rsUse_2     , /////// src2
             dmem_size        , dmem_type         , //////// memory
             md_req_op        , md_req_in_signed_1, //////// multiplication
            md_req_in_signed_2, md_req_out_sel
            },

            {INSN_LEN         , 1                 , IMM_TYPE_WIDTH , ALU_OP_WIDTH   , RS_ENT_SEL,
             1                , ADDR_LEN          , 1              , SPECTAG_LEN    , 1         ,
             REG_SEL          , 1                 ,
             REG_SEL          , SRC_A_SEL_WIDTH   , 1              ,
             REG_SEL          , SRC_B_SEL_WIDTH   , 1              ,
             3                , MEM_TYPE_WIDTH    ,
             MD_OP_WIDTH      , 1                 ,
             1                , MD_OUT_SEL_WIDTH
            }
        };

        inline SlotMeta smDecShard {
            {      pc, bhr        , desEqSrc1, desEqSrc2},
            {ADDR_LEN, GSH_BHR_LEN, 1        , 1        }
        };

        /**
         * RSV
         */
        inline SlotMeta smRsvBase {
            {pc         , rrftag          , rdUse     , aluOp,
             spec       , specTag         ,
             phyIdx_1   , rsSel_1         , rsValid_1 ,
             phyIdx_2   , rsSel_2         , rsValid_2
            },
            ////////////////////////////////////////////////////////////////////
            {ADDR_LEN   , RRF_SEL         , 1          , ALU_OP_WIDTH,
             1          , SPECTAG_LEN     ,
             DATA_LEN   , SRC_A_SEL_WIDTH , 1          ,
             DATA_LEN   , SRC_B_SEL_WIDTH , 1
            }
        };

        inline SlotMeta smRsvBranch{
            {opcode      , pred_addr, imm_br},
            {OPCODE_WIDTH, ADDR_LEN , ADDR_LEN}
        };

        inline SlotMeta smRsvAlu{
            {imm},
            {DATA_LEN}
        };

        inline SlotMeta smRsvMul {
            {md_req_in_signed_1, md_req_in_signed_2, md_req_out_sel   },
            {1                 , 1                 , MD_OUT_SEL_WIDTH }
        };

        inline SlotMeta smRsvO{ /// out of order
            {busy, sortBit},
            {1   , 1      }
        };

        inline SlotMeta smRsvI{
                {busy},
                {1}
        };

        inline SlotMeta smBtb{
                {busy},
                {64}
        };

        inline SlotMeta smStoreBuf{ //// for store buffer entry
                    { busy    ,  complete, spec     , specTag    ,
                      mem_addr
                    },
                    { 1       , 1        , 1        , SPECTAG_LEN,
                      ADDR_LEN
                    }
        };

        inline SlotMeta smLdSt{  //// for second stage system
                    { rrftag   , rdUse   , spec, specTag,
                      stBufData, stBufHit
                    },
                    { RRF_SEL  , 1       , 1   , SPECTAG_LEN,
                      DATA_LEN , 1
                    }
        };

    /**
     * MPFT
     */
    inline SlotMeta smMPFT{
        {mpft_valid, mpft_fixTag},
        {1         , SPECTAG_LEN}
    };

    /**
     * RRF
     */
    inline SlotMeta smRRF{
        {rrfValid, rrfData },
        {1       , DATA_LEN}
    };

    /**
     *  ARF
     */
    ////////// | arfBusy_0 | arfBusy_1 | arfBusy_2 ..... | arfBusy_31
    inline SlotMeta smARFBusy{arfBusy, 1, REG_NUM, 0};
    ////////// | arfRenamed_0 | arfRenamed_1 | arfRenamed_2 ..... | arfRenamed_31
    inline SlotMeta smARFRenamed{arfRenamed, RRF_SEL, REG_NUM, 0};
    ////////// | rrfData_0 | rrfData_1 | rrfData_2 ..... | rrfData_31
    inline SlotMeta smARFData{arfData, DATA_LEN, REG_NUM, 0};

    /**
     * ROB
     */
    inline SlotMeta smROB{    ///// check it
        {wbFin      , isBranch, storeBit, rdUse   , rdIdx  ,
         bhr        , pc      , jumpAddr, jumpCond          },
        /////////////////////////////////////////////////
        {1          , 1       , 1       , 1       , REG_SEL,
         GSH_BHR_LEN, ADDR_LEN, ADDR_LEN, 1       }
    };

}


#endif //KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

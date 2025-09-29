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
            {inst1, invalid1, inst2, invalid2,    pc,   npc, prCond, bhr     },
            {INSN_LEN,           1, INSN_LEN,           1, ADDR_LEN, ADDR_LEN,         1, GSH_BHR_LEN}
        };

        /**
         * DECODE STAGE
         */

        inline SlotMeta smDecBase {
            {invalid       ,imm_type      , aluOp             , rsEnt,
                isBranch      , pred_addr         , spec , specTag, illLegal,
                rdIdx         , rdUse              ,
                rsIdx_1       , rsSel_1           ,rsUse_1 ,
                rsIdx_2       , rsSel_2           ,rsUse_2 ,
                dmem_size     , dmem_type         ,
                md_req_op     , md_req_in_signed_1, md_req_in_signed_2, md_req_out_sel
            },

            {1               , IMM_TYPE_WIDTH  , ALU_OP_WIDTH   , ALU_OP_WIDTH,
                1               , ADDR_LEN       , 1, SPECTAG_LEN , 1,
                REG_SEL         , 1              ,
                REG_SEL         , SRC_A_SEL_WIDTH, 1,
                REG_SEL         , SRC_B_SEL_WIDTH, 1,
                3               , MEM_TYPE_WIDTH ,
                MD_OP_WIDTH     , 1              , 1, MD_OUT_SEL_WIDTH
            }
        };

        inline SlotMeta smDecShard {
            {   pc,      bhr, desEqSrc1, desEqSrc2},
            {ADDR_LEN, GSH_BHR_LEN,            1,            1}
        };

        /**
         * RSV
         */
        inline SlotMeta smRsvBase {
            {
                pc      , imm           , rrftag    , rdUse,     aluOp,  specTag, opcode,
                phyIdx_1, rsSel_1       , rsValid_1 ,
                phyIdx_2, rsSel_2       , rsValid_2 ,
            },
            {ADDR_LEN   ,   DATA_LEN       , RRF_SEL      ,        1, ALU_OP_WIDTH, SPECTAG_LEN, OPCODE_WIDTH,
                DATA_LEN   ,   SRC_A_SEL_WIDTH, 1            ,
                DATA_LEN   ,   SRC_B_SEL_WIDTH, 1            ,
            }
        };

        inline SlotMeta smRsvMul {
            {md_req_in_signed_1, md_req_in_signed_2, md_req_out_sel},
            {                    1,                     1, MD_OUT_SEL_WIDTH }
        };

        inline SlotMeta smRsvOI{ /// out of order
            {busy, sortBit},
            {1, 1}
        };

    /**
     * MPFT
     */
    inline SlotMeta smMPFT{
        {mpft_valid, mpft_fixTag},
        {            1,    SPECTAG_LEN}
    };

    /**
     * RRF
     */
    inline SlotMeta smRRF{
        {rrfValid, rrfData },
        {       1, DATA_LEN}
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



}


#endif //KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

//
// Created by tanawin on 23/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

#include "kathryn.h"
#include "parameter.h"

using namespace std;

namespace kathryn::o3{
    inline SlotMeta buildArchRegSlotMeta(int idx, int selWidth){
            SlotMeta result = {
                {str(rsIdx_) + toS(idx), str(rsUse_) + toS(idx), str(rsSel_) + toS(idx)},
                {REG_SEL               ,                      1,               selWidth}
            };
            return result;
        }

    inline SlotMeta buildSrcPhyRegSlotMeta(int idx, int selWidth){
            SlotMeta result = {
                /// it is shared between phy idx and data
                {str(phyIdx_) + toS(idx), str(rsValid_) + toS(idx), str(rsSel_) + toS(idx)},
                {               DATA_LEN,                      1,               selWidth}
            };
            return result;
        }

        /**
         *   FETCH STAGE
         */

        inline SlotMeta smFetch {
            {inst1, invalid1, inst2, invalid2,    pc,   npc, prCond},
            {INSN_LEN,           1, INSN_LEN,           1, ADDR_LEN, ADDR_LEN,         1}
        };

        /**
         * DECODE STAGE
         */

        inline SlotMeta smDecBase {
            {imm_type      , rdIdx      , rdUse  , aluOp  , rsEnt,
                isBranch      , pred_addr  , spec   , specTag,
                dmem_size     , dmem_type  ,
                md_req_op     , str(md_req_in_signed_) + "1", str(md_req_in_signed_) + "2",
                md_req_out_sel
            },

            {IMM_TYPE_WIDTH  , REG_SEL        , 1, ALU_OP_WIDTH, ALU_OP_WIDTH,
                1               , ADDR_LEN       , 1, SPECTAG_LEN ,
                3               , MEM_TYPE_WIDTH ,
                MD_OP_WIDTH     , 1              , 1,
                MD_OUT_SEL_WIDTH
            }
        };

        inline SlotMeta smDecShard {
            {   pc,      bhr},
            {ADDR_LEN, GSH_BHR_LEN}
        };

        /**
         * RSV
         */
        inline SlotMeta smRsvBase {
            { pc, imm, rrftag, rdUse, aluOp, specTag,       opcode},
            {ADDR_LEN, DATA_LEN, RRF_SEL, 1, ALU_OP_WIDTH, SPECTAG_LEN,    OPCODE_WIDTH}
        };

        inline SlotMeta smRsvMul {
            {str(md_req_in_signed_) + "1", str(md_req_in_signed_) + "2", md_req_out_sel},
            {                           1,                            1, MD_OUT_SEL_WIDTH }
        };

        inline SlotMeta smRsvOI{ /// out of order
            {busy, sortBit},
            {1, 1}
        };


}


#endif //KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

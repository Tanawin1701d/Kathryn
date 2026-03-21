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

        inline SlotMeta sm_fetch {
            {inst1   , inst2   , invalid2,       pc,      npc},
            {INSN_LEN, INSN_LEN, 1       , ADDR_LEN, ADDR_LEN}
        };

        ///inline SlotMeta sm_bhrs{bhr, GSH_BHR_LEN, SPECTAG_LEN, 0};

        /**
         * DECODE STAGE
         */

        inline SlotMeta sm_dec_base {
            {inst              , invalid           , imm_type    , alu_op          , rs_ent, /////// operation
             is_branch          , pred_addr         , spec        , spec_tag        , ill_legal, ////// branch
             rd_idx             , rd_use             ,    /////// destination
             rsIdx_1           , rsSel_1           , rsUse_1     , /////// src1
             rsIdx_2           , rsSel_2           , rsUse_2     , /////// src2
             dmem_size         , dmem_type         , //////// memory
             md_req_op         , md_req_in_signed_1, //////// multiplication
             md_req_in_signed_2, md_req_out_sel
            },

            {INSN_LEN          , 1                 , IMM_TYPE_WIDTH , ALU_OP_WIDTH   , RS_ENT_SEL,
             1                 , ADDR_LEN          , 1              , SPECTAG_LEN    , 1         ,
             REG_SEL           , 1                 ,
             REG_SEL           , SRC_A_SEL_WIDTH   , 1              ,
             REG_SEL           , SRC_B_SEL_WIDTH   , 1              ,
             3                 , MEM_TYPE_WIDTH    ,
             MD_OP_WIDTH       , 1                 ,
             1                 , MD_OUT_SEL_WIDTH
            }
        };

        inline SlotMeta sm_dec_shard {
            {      pc, des_eq_src1, des_eq_src2},
            {ADDR_LEN, 1        , 1        }
        };

        /**
         * RSV
         */
        inline SlotMeta sm_rsv_base {
            {pc         , rrftag          , rd_use     , alu_op,
             spec       , spec_tag         ,
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

        inline SlotMeta sm_rsv_branch{
            {opcode      , pred_addr, imm_br},
            {OPCODE_WIDTH, ADDR_LEN , ADDR_LEN}
        };

        inline SlotMeta sm_rsv_alu{
            {imm},
            {DATA_LEN}
        };

        inline SlotMeta sm_rsv_mul {
            {md_req_in_signed_1, md_req_in_signed_2, md_req_out_sel   },
            {1                 , 1                 , MD_OUT_SEL_WIDTH }
        };

        inline SlotMeta sm_rsv_o{ /// out of order
            {busy, sort_bit},
            {1   , 1      }
        };

        inline SlotMeta sm_rsv_i{
                {busy},
                {1}
        };

        inline SlotMeta sm_btb{
                {busy},
                {64}
        };

        inline SlotMeta sm_store_buf{ //// for store buffer entry
                    { busy    ,  complete, spec     , spec_tag    ,
                      mem_addr
                    },
                    { 1       , 1        , 1        , SPECTAG_LEN,
                      ADDR_LEN
                    }
        };

        inline SlotMeta sm_ld_st{  //// for second stage system
                    { rrftag   , rd_use   , spec, spec_tag,
                      st_buf_data, st_buf_hit
                    },
                    { RRF_SEL  , 1       , 1   , SPECTAG_LEN,
                      DATA_LEN , 1
                    }
        };

    /**
     * MPFT
     */
    inline SlotMeta sm_mpft{
        {mpft_valid, mpft_fixTag},
        {1         , SPECTAG_LEN}
    };

    /**
     * RRF
     */
    inline SlotMeta sm_rrf{
        {rrf_valid, rrf_data },
        {1       , DATA_LEN}
    };

    /**
     *  ARF
     */
    ////////// | arfBusy_0 | arfBusy_1 | arfBusy_2 ..... | arfBusy_31
    inline SlotMeta sm_arf_busy{arf_busy, 1, REG_NUM, 0};
    ////////// | arfRenamed_0 | arfRenamed_1 | arfRenamed_2 ..... | arfRenamed_31
    inline SlotMeta sm_arf_renamed{arf_renamed, RRF_SEL, REG_NUM, 0};
    ////////// | rrfData_0 | rrfData_1 | rrfData_2 ..... | rrfData_31
    inline SlotMeta sm_arf_data{arf_data, DATA_LEN, REG_NUM, 0};

    /**
     * ROB
     */
    inline SlotMeta sm_rob{    ///// check it
        {wb_fin, is_branch, store_bit,
         rd_use, rd_idx   , pc       },
        /////////////////////////////////////////////////
        {1    , 1       , 1       ,
         1    , REG_SEL , ADDR_LEN}
    };

}


#endif //KATHRYN_SRC_EXAMPLE_O3_BLKPARAM_H

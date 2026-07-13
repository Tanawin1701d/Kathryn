//
// Created by tanawin on 23/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_PARAMETER_H
#define KATHRYN_SRC_EXAMPLE_O3_PARAMETER_H

#include "kathryn.h"
#include "isaParam.h"

namespace kathryn::o3{

#define str(name) std::string(name)   ///HLH CORE
#define toS(value) std::to_string(value)   ///HLH CORE
#define O3_PARAM_STR(name) constexpr char name[] = #name;   ///HLH CORE

///#define BTB_ENABLE

    ////// start from fetch
    O3_PARAM_STR(pc);   ///PARAM CORE
    O3_PARAM_STR(npc);   ///PARAM CORE

    O3_PARAM_STR(inst1);   ///PARAM CORE
    O3_PARAM_STR(invalid1);   ///PARAM CORE
    O3_PARAM_STR(inst2);   ///PARAM CORE
    O3_PARAM_STR(invalid2);   ///PARAM CORE
    ////// start from decoder

    O3_PARAM_STR(inst);   ///PARAM CORE
    O3_PARAM_STR(imm_type);   ///PARAM CORE

    O3_PARAM_STR(rsIdx_);   ///PARAM CORE
    O3_PARAM_STR(rsIdx_1);   ///PARAM CORE
    O3_PARAM_STR(rsIdx_2);   ///PARAM CORE
    O3_PARAM_STR(rsUse_);   ///PARAM CORE
    O3_PARAM_STR(rsUse_1);   ///PARAM CORE
    O3_PARAM_STR(rsUse_2);   ///PARAM CORE
    O3_PARAM_STR(rsValid_);   ///PARAM CORE
    O3_PARAM_STR(rsValid_1);   ///PARAM CORE
    O3_PARAM_STR(rsValid_2);   ///PARAM CORE
    O3_PARAM_STR(rsSel_);   ///PARAM CORE
    O3_PARAM_STR(rsSel_1);   ///PARAM CORE
    O3_PARAM_STR(rsSel_2);   ///PARAM CORE

    O3_PARAM_STR(invalid);   ///PARAM CORE


    O3_PARAM_STR(rdIdx);   ///PARAM CORE
    O3_PARAM_STR(rdUse);   ///PARAM CORE

    O3_PARAM_STR(aluOp);   ///PARAM CORE
    O3_PARAM_STR(opcode);   ///PARAM CORE

    O3_PARAM_STR(isBranch);   ///PARAM CORE
    O3_PARAM_STR(pred_addr);   ///PARAM CORE
    O3_PARAM_STR(spec);   ///PARAM CORE
    O3_PARAM_STR(specTag);   ///PARAM CORE

    /// load store buffer
    O3_PARAM_STR(complete);   ///PARAM CORE
    O3_PARAM_STR(mem_addr);   ///PARAM CORE

    O3_PARAM_STR(busy);   ///PARAM CORE
    O3_PARAM_STR(sortBit);   ///PARAM CORE


    O3_PARAM_STR(rsEnt);   ///PARAM CORE

    O3_PARAM_STR(rs_valid_);   ///PARAM CORE

    O3_PARAM_STR(illLegal);   ///PARAM CORE

    O3_PARAM_STR(dmem_size);   ///PARAM CORE
    O3_PARAM_STR(dmem_type);   ///PARAM CORE

    O3_PARAM_STR(md_req_op);   ///PARAM CORE
    O3_PARAM_STR(md_req_in_signed_);   ///PARAM CORE
    O3_PARAM_STR(md_req_in_signed_1);   ///PARAM CORE
    O3_PARAM_STR(md_req_in_signed_2);   ///PARAM CORE
    O3_PARAM_STR(md_req_out_sel);   ///PARAM CORE

    O3_PARAM_STR(desEqSrc1);   ///PARAM CORE
    O3_PARAM_STR(desEqSrc2);   ///PARAM CORE
    ////// start from ldsdt
    O3_PARAM_STR(stBufData); //// the data from   ///PARAM CORE
    O3_PARAM_STR(stBufHit);   ///PARAM CORE
    ////// start from rsv
    O3_PARAM_STR(imm);   ///PARAM CORE
    O3_PARAM_STR(imm_br);   ///PARAM CORE
    O3_PARAM_STR(rrftag);   ///PARAM CORE
    O3_PARAM_STR(phyIdx_);   ///PARAM CORE
    O3_PARAM_STR(phyIdx_1);   ///PARAM CORE
    O3_PARAM_STR(phyIdx_2);   ///PARAM CORE
    O3_PARAM_STR(rs1);   ///PARAM CORE
    O3_PARAM_STR(entry_ready);   ///PARAM CORE

    ///// for mpft
    O3_PARAM_STR(mpft_valid)   ///PARAM CORE
    O3_PARAM_STR(mpft_fixTag)   ///PARAM CORE
    //// for RRF
    O3_PARAM_STR(rrfValid);   ///PARAM CORE
    O3_PARAM_STR(rrfData);   ///PARAM CORE
    //// for ARF
    O3_PARAM_STR(arfBusy);   ///PARAM CORE
    O3_PARAM_STR(arfRenamed);   ///PARAM CORE
    O3_PARAM_STR(arfData);   ///PARAM CORE
    //// for ROB
    O3_PARAM_STR(wbFin);   ///PARAM CORE
    O3_PARAM_STR(storeBit);   ///PARAM CORE

    //// for Exec Unit
    O3_PARAM_STR(bpValid);   ///PARAM CORE


        ////// rsIdx use with data in the system

///// IMEM parameter
constexpr int IMEM_IDX_WIDTH = 11;   ///PARAM CORE
constexpr int IMEM_ROW   = 1 << IMEM_IDX_WIDTH;   ///PARAM CORE
constexpr int IMEM_WIDTH = 32;   ///PARAM CORE

///// DMEM parameter
constexpr int DMEM_IDX_WIDTH = 11;   ///PARAM CORE
constexpr int DMEM_ROW   = 1 << DMEM_IDX_WIDTH;   ///PARAM CORE
constexpr int DMEM_WIDTH = 32;   ///PARAM CORE


//Register File
constexpr int REG_SEL =  5;   ///PARAM CORE
//constexpr int REG_NUM 2**`REG_SEL
constexpr int REG_NUM =  32;   ///PARAM CORE

// opcode width
constexpr int OPCODE_WIDTH = 7;   ///PARAM CORE
//Instruction
constexpr int IMM_TYPE_WIDTH = 2;   ///PARAM CORE
constexpr int IMM_I =0;   ///PARAM CORE
constexpr int IMM_S =1;   ///PARAM CORE
constexpr int IMM_U =2;   ///PARAM CORE
constexpr int IMM_J =3;   ///PARAM CORE

//Important Wire
constexpr int DATA_LEN  = 32;   ///PARAM CORE
constexpr int INSN_LEN  = 32;   ///PARAM CORE
constexpr int ADDR_LEN  = 32;   ///PARAM CORE
constexpr int ISSUE_NUM =  2;   ///PARAM CORE
constexpr int ENTRY_POINT = 0;   ///PARAM CORE
//constexpr int REQDATA_LEN 2

//Decoder
constexpr int RS_ENT_SEL    = 3;   ///PARAM CORE
constexpr int RS_ENT_ALU    = 1;   ///PARAM CORE
constexpr int RS_ENT_BRANCH = 2;   ///PARAM CORE
constexpr int RS_ENT_JAL    = RS_ENT_BRANCH;   ///PARAM CORE
constexpr int RS_ENT_JALR   = RS_ENT_BRANCH;   ///PARAM CORE
constexpr int RS_ENT_MUL    = 3;   ///PARAM CORE
constexpr int RS_ENT_DIV    = 3;   ///PARAM CORE
constexpr int RS_ENT_LDST   = 4;   ///PARAM CORE

//RS
constexpr int ALU_ENT_SEL    = 3;   ///PARAM CORE
constexpr int ALU_ENT_NUM    = 8;   ///PARAM CORE
constexpr int BRANCH_ENT_SEL = 2;   ///PARAM CORE
constexpr int BRANCH_ENT_NUM = 4;   ///PARAM CORE
constexpr int LDST_ENT_SEL   = 2;   ///PARAM CORE
constexpr int LDST_ENT_NUM   = 4;   ///PARAM CORE
//constexpr int LDST_ENT_SEL 3
//constexpr int LDST_ENT_NUM 8
constexpr int MUL_ENT_SEL = 1;   ///PARAM CORE
constexpr int MUL_ENT_NUM = 2;   ///PARAM CORE

//STOREBUFFER
constexpr int STBUF_ENT_SEL =  5;   ///PARAM CORE
constexpr int STBUF_ENT_NUM =  32;   ///PARAM CORE

//BTB
constexpr int BTB_IDX_SEL = 9;   ///PARAM CORE
constexpr int BTB_IDX_NUM = 512;   ///PARAM CORE

constexpr int BTB_IDX_SEL_START = 3;   ///PARAM CORE
constexpr int BTB_IDX_SEL_STOP  = BTB_IDX_SEL_START + BTB_IDX_SEL;   ///PARAM CORE
//constexpr int BTB_IDX_NUM 2**`BTB_IDX_SEL
//constexpr int BTB_TAG_LEN `ADDR_LEN-3-`BTB_IDX_SEL
constexpr int BTB_TAG_LEN = 20;   ///PARAM CORE

//Gshare
constexpr int GSH_BHR_LEN = 10;   ///PARAM CORE
constexpr int GSH_BHR_ST_IDX = 2;   ///PARAM CORE
constexpr int GSH_BHR_END_IDX = GSH_BHR_ST_IDX + GSH_BHR_LEN;   ///PARAM CORE
constexpr int GSH_PHT_SEL = 10;   ///PARAM CORE
constexpr int GSH_PHT_NUM = 1024;   ///PARAM CORE
//constexpr int GSH_PHT_NUM 2**`GSH_PHT_SEL

//TagGenerator

//constexpr int SPECTAG_LEN 1+`BRANCH_ENT_NUM
constexpr int SPECTAG_LEN = 5;   ///PARAM CORE
//constexpr int BRDEPTH_LEN `SPECTAG_LEN
constexpr int BRDEPTH_LEN = 5;   ///PARAM CORE

//Re-Order Buffer
constexpr int ROB_SEL = 6;   ///PARAM CORE
//constexpr int ROB_NUM 2**`ROB_SEL
constexpr int ROB_NUM = 64;   ///PARAM CORE
constexpr int RRF_SEL = ROB_SEL;   ///PARAM CORE
constexpr int RRF_NUM = ROB_NUM;   ///PARAM CORE

//src_a
constexpr int SRC_A_SEL_WIDTH = 2;   ///PARAM CORE
constexpr int SRC_A_RS1       = 0;   ///PARAM CORE
constexpr int SRC_A_PC        = 1;   ///PARAM CORE
constexpr int SRC_A_ZERO      = 2;   ///PARAM CORE

//src_b
constexpr int SRC_B_SEL_WIDTH = 2;   ///PARAM CORE
constexpr int SRC_B_RS2       = 0;   ///PARAM CORE
constexpr int SRC_B_IMM       = 1;   ///PARAM CORE
constexpr int SRC_B_FOUR      = 2;   ///PARAM CORE
constexpr int SRC_B_ZERO      = 3;   ///PARAM CORE

//load
constexpr int MEM_TYPE_WIDTH  = 3;   ///PARAM CORE
constexpr int MEM_TYPE_LB     = 0;   ///PARAM CORE
constexpr int MEM_TYPE_LH     = 1;   ///PARAM CORE
constexpr int MEM_TYPE_LW     = 2;   ///PARAM CORE
constexpr int MEM_TYPE_LD     = 3;   ///PARAM CORE
constexpr int MEM_TYPE_LBU    = 4;   ///PARAM CORE
constexpr int MEM_TYPE_LHU    = 5;   ///PARAM CORE
constexpr int MEM_TYPE_LWU    = 6;   ///PARAM CORE

constexpr int MEM_TYPE_SB     = 0;   ///PARAM CORE
constexpr int MEM_TYPE_SH     = 1;   ///PARAM CORE
constexpr int MEM_TYPE_SW     = 2;   ///PARAM CORE
constexpr int MEM_TYPE_SD     = 3;   ///PARAM CORE

constexpr int MD_OP_WIDTH     = 2;   ///PARAM CORE
constexpr int MD_OP_MUL       = 0;   ///PARAM CORE
constexpr int MD_OP_DIV       = 1;   ///PARAM CORE
constexpr int MD_OP_REM       = 2;   ///PARAM CORE

constexpr int MD_OUT_SEL_WIDTH = 2;   ///PARAM CORE
constexpr int MD_OUT_LO        = 0;   ///PARAM CORE
constexpr int MD_OUT_HI        = 1;   ///PARAM CORE
constexpr int MD_OUT_REM       = 2;   ///PARAM CORE

constexpr int ALU_OP_WIDTH = 4;   ///PARAM CORE

constexpr int ALU_OP_ADD  = 0;   ///PARAM CORE
constexpr int ALU_OP_SLL  = 1;   ///PARAM CORE
constexpr int ALU_OP_XOR  = 4;   ///PARAM CORE
constexpr int ALU_OP_OR   = 6;   ///PARAM CORE
constexpr int ALU_OP_AND  = 7;   ///PARAM CORE
constexpr int ALU_OP_SRL  = 5;   ///PARAM CORE
constexpr int ALU_OP_SEQ  = 8;   ///PARAM CORE
constexpr int ALU_OP_SNE  = 9;   ///PARAM CORE
constexpr int ALU_OP_SUB  = 10;   ///PARAM CORE
constexpr int ALU_OP_SRA  = 11;   ///PARAM CORE
constexpr int ALU_OP_SLT  = 12;   ///PARAM CORE
constexpr int ALU_OP_SGE  = 13;   ///PARAM CORE
constexpr int ALU_OP_SLTU = 14;   ///PARAM CORE
constexpr int ALU_OP_SGEU = 15;   ///PARAM CORE

}

#endif //KATHRYN_PARAMETER_H

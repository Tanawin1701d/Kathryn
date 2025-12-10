//
// Created by tanawin on 23/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_PARAMETER_H
#define KATHRYN_SRC_EXAMPLE_O3_PARAMETER_H

#include <string>
#include <string_view>

namespace kathryn::o3{

#define str(name) std::string(name)
#define toS(value) std::to_string(value)
#define O3_PARAM_STR(name) constexpr char name[] = #name;

    ////// start from fetch
    O3_PARAM_STR(pc);
    O3_PARAM_STR(npc);

    O3_PARAM_STR(inst1);
    O3_PARAM_STR(invalid1);
    O3_PARAM_STR(inst2);
    O3_PARAM_STR(invalid2);
    O3_PARAM_STR(bhr);
    O3_PARAM_STR(prCond);
    ////// start from decoder

    O3_PARAM_STR(inst);
    O3_PARAM_STR(imm_type);

    O3_PARAM_STR(rsIdx_);
    O3_PARAM_STR(rsIdx_1);
    O3_PARAM_STR(rsIdx_2);
    O3_PARAM_STR(rsUse_);
    O3_PARAM_STR(rsUse_1);
    O3_PARAM_STR(rsUse_2);
    O3_PARAM_STR(rsValid_);
    O3_PARAM_STR(rsValid_1);
    O3_PARAM_STR(rsValid_2);
    O3_PARAM_STR(rsSel_);
    O3_PARAM_STR(rsSel_1);
    O3_PARAM_STR(rsSel_2);

    O3_PARAM_STR(invalid);


    O3_PARAM_STR(rdIdx);
    O3_PARAM_STR(rdUse);

    O3_PARAM_STR(aluOp);
    O3_PARAM_STR(opcode);

    O3_PARAM_STR(isBranch);
    O3_PARAM_STR(pred_addr);
    O3_PARAM_STR(spec);
    O3_PARAM_STR(specTag);

    /// load store buffer
    O3_PARAM_STR(complete);
    O3_PARAM_STR(mem_addr);

    O3_PARAM_STR(busy);
    O3_PARAM_STR(sortBit);


    O3_PARAM_STR(rsEnt);

    O3_PARAM_STR(rs_valid_);

    O3_PARAM_STR(illLegal);

    O3_PARAM_STR(dmem_size);
    O3_PARAM_STR(dmem_type);

    O3_PARAM_STR(md_req_op);
    O3_PARAM_STR(md_req_in_signed_);
    O3_PARAM_STR(md_req_in_signed_1);
    O3_PARAM_STR(md_req_in_signed_2);
    O3_PARAM_STR(md_req_out_sel);

    O3_PARAM_STR(desEqSrc1);
    O3_PARAM_STR(desEqSrc2);
    ////// start from ldsdt
    O3_PARAM_STR(stBufData); //// the data from
    O3_PARAM_STR(stBufHit);
    ////// start from rsv
    O3_PARAM_STR(imm);
    O3_PARAM_STR(imm_br);
    O3_PARAM_STR(rrftag);
    O3_PARAM_STR(phyIdx_);
    O3_PARAM_STR(phyIdx_1);
    O3_PARAM_STR(phyIdx_2);
    O3_PARAM_STR(rs1);
    O3_PARAM_STR(entry_ready);

    ///// for mpft
    O3_PARAM_STR(mpft_valid)
    O3_PARAM_STR(mpft_fixTag)
    //// for RRF
    O3_PARAM_STR(rrfValid);
    O3_PARAM_STR(rrfData);
    //// for ARF
    O3_PARAM_STR(arfBusy);
    O3_PARAM_STR(arfRenamed);
    O3_PARAM_STR(arfData);
    //// for ROB
    O3_PARAM_STR(wbFin);

    //// for Exec Unit
    O3_PARAM_STR(bpValid);


        ////// rsIdx use with data in the system

constexpr int IMEM_IDX_WIDTH = 10;
constexpr int IMEM_ROW   = 1 << IMEM_IDX_WIDTH;
constexpr int IMEM_WIDTH = 32;


//Register File
constexpr int REG_SEL =  5;
//constexpr int REG_NUM 2**`REG_SEL
constexpr int REG_NUM =  32;

// opcode width
constexpr int OPCODE_WIDTH = 7;
//Instruction
constexpr int IMM_TYPE_WIDTH = 2;
constexpr int IMM_I =0;
constexpr int IMM_S =1;
constexpr int IMM_U =2;
constexpr int IMM_J =3;

//Important Wire
constexpr int DATA_LEN  = 32;
constexpr int INSN_LEN  = 32;
constexpr int ADDR_LEN  = 32;
constexpr int ISSUE_NUM =  2;
constexpr int ENTRY_POINT = 0;
//constexpr int REQDATA_LEN 2

//Decoder
constexpr int RS_ENT_SEL    = 3;
constexpr int RS_ENT_ALU    = 1;
constexpr int RS_ENT_BRANCH = 2;
constexpr int RS_ENT_JAL    = RS_ENT_BRANCH;
constexpr int RS_ENT_JALR   = RS_ENT_BRANCH;
constexpr int RS_ENT_MUL    = 3;
constexpr int RS_ENT_DIV    = 3;
constexpr int RS_ENT_LDST   = 4;

//RS
constexpr int ALU_ENT_SEL    = 3;
constexpr int ALU_ENT_NUM    = 8;
constexpr int BRANCH_ENT_SEL = 2;
constexpr int BRANCH_ENT_NUM = 4;
constexpr int LDST_ENT_SEL   = 2;
constexpr int LDST_ENT_NUM   = 4;
//constexpr int LDST_ENT_SEL 3
//constexpr int LDST_ENT_NUM 8
constexpr int MUL_ENT_SEL = 1;
constexpr int MUL_ENT_NUM = 2;

//STOREBUFFER
constexpr int STBUF_ENT_SEL =  5;
constexpr int STBUF_ENT_NUM =  32;

//BTB
constexpr int BTB_IDX_SEL = 9;
constexpr int BTB_IDX_NUM = 512;
//constexpr int BTB_IDX_NUM 2**`BTB_IDX_SEL
//constexpr int BTB_TAG_LEN `ADDR_LEN-3-`BTB_IDX_SEL
constexpr int BTB_TAG_LEN = 20;

//Gshare
constexpr int GSH_BHR_LEN = 10;
constexpr int GSH_PHT_SEL = 10;
constexpr int GSH_PHT_NUM = 1024;
//constexpr int GSH_PHT_NUM 2**`GSH_PHT_SEL

//TagGenerator

//constexpr int SPECTAG_LEN 1+`BRANCH_ENT_NUM
constexpr int SPECTAG_LEN = 5;
//constexpr int BRDEPTH_LEN `SPECTAG_LEN
constexpr int BRDEPTH_LEN = 5;

//Re-Order Buffer
constexpr int ROB_SEL = 6;
//constexpr int ROB_NUM 2**`ROB_SEL
constexpr int ROB_NUM = 64;
constexpr int RRF_SEL = ROB_SEL;
constexpr int RRF_NUM = ROB_NUM;

//src_a
constexpr int SRC_A_SEL_WIDTH = 2;
constexpr int SRC_A_RS1       = 0;
constexpr int SRC_A_PC        = 1;
constexpr int SRC_A_ZERO      = 2;

//src_b
constexpr int SRC_B_SEL_WIDTH = 2;
constexpr int SRC_B_RS2       = 0;
constexpr int SRC_B_IMM       = 1;
constexpr int SRC_B_FOUR      = 2;
constexpr int SRC_B_ZERO      = 3;

//load
constexpr int MEM_TYPE_WIDTH  = 3;
constexpr int MEM_TYPE_LB     = 0;
constexpr int MEM_TYPE_LH     = 1;
constexpr int MEM_TYPE_LW     = 2;
constexpr int MEM_TYPE_LD     = 3;
constexpr int MEM_TYPE_LBU    = 4;
constexpr int MEM_TYPE_LHU    = 5;
constexpr int MEM_TYPE_LWU    = 6;

constexpr int MEM_TYPE_SB     = 0;
constexpr int MEM_TYPE_SH     = 1;
constexpr int MEM_TYPE_SW     = 2;
constexpr int MEM_TYPE_SD     = 3;

constexpr int MD_OP_WIDTH     = 2;
constexpr int MD_OP_MUL       = 0;
constexpr int MD_OP_DIV       = 1;
constexpr int MD_OP_REM       = 2;

constexpr int MD_OUT_SEL_WIDTH = 2;
constexpr int MD_OUT_LO        = 0;
constexpr int MD_OUT_HI        = 1;
constexpr int MD_OUT_REM       = 2;

constexpr int ALU_OP_WIDTH = 4;

constexpr int ALU_OP_ADD  = 0;
constexpr int ALU_OP_SLL  = 1;
constexpr int ALU_OP_XOR  = 4;
constexpr int ALU_OP_OR   = 6;
constexpr int ALU_OP_AND  = 7;
constexpr int ALU_OP_SRL  = 5;
constexpr int ALU_OP_SEQ  = 8;
constexpr int ALU_OP_SNE  = 9;
constexpr int ALU_OP_SUB  = 10;
constexpr int ALU_OP_SRA  = 11;
constexpr int ALU_OP_SLT  = 12;
constexpr int ALU_OP_SGE  = 13;
constexpr int ALU_OP_SLTU = 14;
constexpr int ALU_OP_SGEU = 15;

}

#endif //KATHRYN_PARAMETER_H
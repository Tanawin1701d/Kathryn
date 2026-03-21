//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DECODER_H
#define KATHRYN_SRC_EXAMPLE_O3_DECODER_H

#include "kathryn.h"
#include "stage_struct.h"
#include "example/o3/simulation/prober_grp.h"  ///DC


namespace kathryn::o3{

    struct DecMod: Module{
        PipStage& pm;
        TagMgmt& tag_mgmt;

        m_wire(dbg_isGenable, 1); ///DC

        explicit DecMod(
        PipStage&  pm,
        TagMgmt&   tag_mg) :
            pm(pm),
            tag_mgmt(tag_mg){
            pm.dc.dcd1     .make_reset_event();
            pm.dc.dcd2     .make_reset_event();
            pm.dc.dcd_shared.make_reset_event();
        }

        ////// dcd = decoded
        void  decode(int idx){ //// idx start at 1 the second is 2

            bool is_first = (idx == 1);
            RegSlot&  raw       = pm.ft.raw;
            Reg&      instr     = is_first? pm.ft.raw(inst1)   : pm.ft.raw(inst2);
            WireSlot& dcw_first  = pm.dc.dcw1;
            WireSlot& dcw       = is_first? pm.dc.dcw1 : pm.dc.dcw2;

            m_val(invalid1_dummy, 1, 0);
            opr&      inv   = is_first? (opr&) invalid1_dummy :
                                       (opr&) pm.ft.raw(invalid2);

            ///////////// src
            Operable& opc     = instr( 0,  7);
            Operable& funct7  = instr(25, 32);
            Operable& funct12 = instr(20, 32);
            Operable& funct3  = instr(12, 15);

            ///////////// des
            dcw(inst)      = instr;
            dcw(imm_type)  = IMM_I;
            dcw(alu_op)     = ALU_OP_ADD;
            dcw(rs_ent)     = RS_ENT_ALU;
            dcw(is_branch)  = 0;
            dcw(ill_legal)  = 0;
            ///////////// calculate address and validation
            if (idx == 1){
                dcw(invalid)   = 0; ///// we are sure about first instruction
                dcw(pred_addr) = raw(pc) + 4; //// if the decoded show it is branch or jump it will be overrided
            }else{
                /// if the first one is the branch and predict taken we
                dcw(invalid)   = raw(invalid2);
                dcw(pred_addr) = raw(npc);
            }


            /////// register translator
            dcw(rd_idx)     = instr(7, 12);
            dcw(rd_use)     = 0;
            /////// srca
            dcw(rsIdx_1)   = instr(15, 20);
            dcw(rsUse_1) = 1;
            dcw(rsSel_1)   = SRC_A_RS1;
            /////// srcb
            dcw(rsIdx_2)   = instr(20, 25);
            dcw(rsUse_2)   = 0;
            dcw(rsSel_2)   = SRC_B_IMM;

            m_wire(alu_op_arith, ALU_OP_WIDTH);


            ztate(opc){
                zcase(RV32_LOAD){
                    dcw(rd_use) = 1;
                    dcw(rs_ent) = RS_ENT_LDST;
                }
                zcase( RV32_STORE){
                    dcw(imm_type) = IMM_S;
                    dcw(rs_ent)    = RS_ENT_LDST;
                    dcw(rsUse_2)  = 1;
                }
                zcase(RV32_BRANCH){
                    ztate(funct3){
                        zcase (RV32_FUNCT3_BEQ ){ dcw(alu_op) = ALU_OP_SEQ; }
                        zcase (RV32_FUNCT3_BNE ){ dcw(alu_op) = ALU_OP_SNE; }
                        zcase (RV32_FUNCT3_BLT ){ dcw(alu_op) = ALU_OP_SLT; }
                        zcase (RV32_FUNCT3_BLTU){ dcw(alu_op) = ALU_OP_SLTU;}
                        zcase (RV32_FUNCT3_BGE ){ dcw(alu_op) = ALU_OP_SGE; }
                        zcase (RV32_FUNCT3_BGEU){ dcw(alu_op) = ALU_OP_SGEU;}
                        zcasedef{dcw(ill_legal) = 1;}
                    }

                    dcw(rs_ent)   = RS_ENT_BRANCH;
                    dcw(rsUse_2) = 1;
                    dcw(rsSel_2) = SRC_B_RS2;

                    ///// calculate the next address
                    dcw(is_branch) = ~inv;
                }
                zcase(RV32_JAL){
                    dcw(rs_ent)    = RS_ENT_JAL;
                    dcw(rsUse_1)  = 0;
                    dcw(rsSel_1)  = SRC_A_PC;
                    dcw(rsSel_2)  = SRC_B_FOUR;
                    dcw(rd_use)    = 1;
                    dcw(is_branch) = ~inv;
                }
                zcase(RV32_JALR){
                    dcw(ill_legal) = (funct3 != 0);
                    dcw(rs_ent)    = RS_ENT_JALR;
                    dcw(rsSel_1)  = SRC_A_PC;
                    dcw(rsSel_2)  = SRC_B_FOUR;
                    dcw(rd_use)    = 1;
                    dcw(is_branch) = ~inv;
                }
                zcase(RV32_OP_IMM){
                    dcw(alu_op) = alu_op_arith;
                    dcw(rd_use) = 1;
                }
                zcase(RV32_OP){
                    dcw(alu_op)   = alu_op_arith;
                    dcw(rd_use) = 1;
                    dcw(rsUse_2) = 1;
                    dcw(rsSel_2) = SRC_B_RS2;
                    zif( (funct7 == RV32_FUNCT7_MUL_DIV) ){
                        zif ((funct3 == RV32_FUNCT3_MUL) ||
                             (funct3 == RV32_FUNCT3_MULH) ||
                             (funct3 == RV32_FUNCT3_MULHSU) ||
                             (funct3 == RV32_FUNCT3_MULHU)){
                            dcw(rs_ent) = RS_ENT_MUL;
                        }zelse{
                            dcw(rs_ent) = RS_ENT_DIV;
                        }
                    }
                }
                zcase(RV32_AUIPC){
                    dcw(imm_type)  = IMM_U;
                    dcw(rd_use   )  = 1;
                    dcw(rsUse_1 )  = 0;
                    dcw(rsSel_1 )  = SRC_A_PC;
                }
                zcase(RV32_LUI){
                    dcw(imm_type) = IMM_U;
                    dcw(rd_use   ) = 1;
                    dcw(rsUse_1 ) = 0;
                    dcw(rsSel_1 ) = SRC_A_ZERO;
                }
                zcasedef{
                    dcw(ill_legal) = 1;
                }
            }

            ztate(funct3){
                zcase(RV32_FUNCT3_ADD_SUB){
                    alu_op_arith = ALU_OP_ADD;
                    zif (opc == RV32_OP && funct7.sl(5)){
                        alu_op_arith = ALU_OP_SUB;
                    }
                }
                zcase(RV32_FUNCT3_SLL    ){alu_op_arith = ALU_OP_SLL;}
                zcase(RV32_FUNCT3_SLT    ){alu_op_arith = ALU_OP_SLT;}
                zcase(RV32_FUNCT3_SLTU   ){alu_op_arith = ALU_OP_SLTU;}
                zcase(RV32_FUNCT3_XOR    ){alu_op_arith = ALU_OP_XOR;}
                zcase(RV32_FUNCT3_SRA_SRL){
                    zif(funct7.sl(5)){ alu_op_arith = ALU_OP_SRA;}
                    zelse            { alu_op_arith = ALU_OP_SRL;}
                }
                zcase(RV32_FUNCT3_OR     ){alu_op_arith = ALU_OP_OR;}
                zcase(RV32_FUNCT3_AND    ){alu_op_arith = ALU_OP_AND;}
                zcasedef{alu_op_arith = ALU_OP_ADD;}
            }

            /////// multiplier

            dcw(md_req_op)          = MD_OP_MUL;
            dcw(md_req_in_signed_1) = 0;
            dcw(md_req_in_signed_2) = 0;
            dcw(md_req_out_sel)     = MD_OUT_LO;


            ztate(funct3){
                // zcase(RV32_FUNCT3_MUL){
                //     // Default values are already set
                // }
                zcase(RV32_FUNCT3_MULH){
                    dcw(md_req_in_signed_1) = 1;
                    dcw(md_req_in_signed_2) = 1;
                    dcw(md_req_out_sel) = MD_OUT_HI;
                }zcase(RV32_FUNCT3_MULHSU){
                    dcw(md_req_in_signed_1) = 1;
                    dcw(md_req_out_sel) = MD_OUT_HI;
                }zcase(RV32_FUNCT3_MULHU){
                    dcw(md_req_out_sel) = MD_OUT_HI;
                }zcase(RV32_FUNCT3_DIV){
                    dcw(md_req_op) = MD_OP_DIV;
                    dcw(md_req_in_signed_1) = 1;
                    dcw(md_req_in_signed_2) = 1;
                }zcase(RV32_FUNCT3_DIVU){
                    dcw(md_req_op) = MD_OP_DIV;
                }zcase(RV32_FUNCT3_REM){
                    dcw(md_req_op) = MD_OP_REM;
                    dcw(md_req_in_signed_1) = 1;
                    dcw(md_req_in_signed_2) = 1;
                    dcw(md_req_out_sel) = MD_OUT_REM;
                }zcase(RV32_FUNCT3_REMU) {
                    dcw(md_req_op) = MD_OP_REM;
                    dcw(md_req_out_sel) = MD_OUT_REM;
                }
            }
        }


        void flow() override{
            ///// build the decode wire
            decode(1);
            decode(2);
            ///// reference the two lane of decoded register
            RegSlot&  dcd1 = pm.dc.dcd1;
            RegSlot&  dcd2 = pm.dc.dcd2;
            RegSlot&  dcd_shared = pm.dc.dcd_shared;
            WireSlot& dcw1 = pm.dc.dcw1;
            WireSlot& dcw2 = pm.dc.dcw2;

            ///// check the tag generator is capable to gen the new address tag
            opr& is_genable = tag_mgmt.tag_gen.is_all_genble(
                dcw1(is_branch), //// is_branch will set when invalid is false and the instruction is jumping instruction
                dcw2(is_branch));

            dbg_isGenable = is_genable; ///DC

            pip(pm.dc.sync){                    init_probe(pip_prob_grp .decode); ///CTRL DECODE
                zyncc(pm.sync_dp, is_genable){   init_probe(zync_prob_grp.decode); ///CTRL DECODE
                    ///////// decoded value (except spec_tag and spec)
                    dcd1 <<= dcw1;
                    dcd2 <<= dcw2;
                    dcd_shared(pc)  <<= pm.ft.raw(pc);

                    dcd_shared(des_eq_src1) <<=
                        ((dcw2(rsIdx_1) == dcw1(rd_idx)) & dcw1(rd_use));
                    dcd_shared(des_eq_src2) <<=
                        ((dcw2(rsIdx_2) == dcw1(rd_idx)) & dcw1(rd_use));

                    ///////// generate the tag
                    auto[gen_tag1, gen_tag2] =
                    tag_mgmt.tag_gen.allocate(   //// the tag_gen
                    dcw1(is_branch),dcd1(spec), ///// (is_branch) is input
                    dcw2(is_branch),dcd2(spec)  ///// (spec) taggen will update it
                    );
                    ///// assign decoded data
                    dcd1(spec_tag) <<= gen_tag1;
                    dcd2(spec_tag) <<= gen_tag2;
                    ///// update the mpft
                    tag_mgmt.mpft.on_add_new(
                        dcw1(is_branch), gen_tag1,
                        dcw2(is_branch), gen_tag2
                    );
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_DECODER_H
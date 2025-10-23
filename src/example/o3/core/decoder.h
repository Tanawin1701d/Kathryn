//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DECODER_H
#define KATHRYN_SRC_EXAMPLE_O3_DECODER_H

#include "kathryn.h"
#include "parameter.h"
#include "stageStruct.h"
#include "isaParam.h"
#include "example/o3/simulation/proberGrp.h"


namespace kathryn::o3{

    struct DecMod: Module{
        PipStage& pm;
        TagMgmt& tagMgmt;

        mWire(dbg_isGenable, 1);

        explicit DecMod(
        PipStage&  pm,
        TagMgmt&   tagMg) :
            pm(pm),
            tagMgmt(tagMg){}

        ////// dcd = decoded
        void  decode(int idx){ //// idx start at 1

            bool isFirst = (idx == 1);
            RegSlot&  raw = pm.ft.raw;
            Reg&      instr = isFirst? pm.ft.raw(inst1)   : pm.ft.raw(inst2);
            WireSlot& dcw   = isFirst? pm.dc.dcw1 : pm.dc.dcw2;
            Reg&      inv   = isFirst? pm.ft.raw(invalid1) : pm.ft.raw(invalid2);


            ///////////// src
            Operable& opc     = instr( 0,  7);
            Operable& funct7  = instr(25, 32);
            Operable& funct12 = instr(20, 32);
            Operable& funct3  = instr(12, 15);

            ///////////// des
            dcw(inst)      = instr;
            dcw(imm_type)  = IMM_I;
            dcw(aluOp)     = ALU_OP_ADD;
            dcw(rsEnt)     = RS_ENT_ALU;
            dcw(isBranch)  = 0;
            dcw(illLegal)  = 0;
            ///////////// calculate address and validation
            if (idx == 1){
                dcw(invalid)   = 0; ///// we are sure about first instruction
                dcw(pred_addr) = (raw(pc) + 4); //// if the decoded show it is branch or jump it will be overrided
            }else{
                dcw(invalid)   = raw(invalid2);
                dcw(pred_addr) = raw(npc);
            }


            /////// register translator
            dcw(rdIdx)     = instr(7, 12);
            dcw(rdUse)     = 0;
            /////// srca
            dcw(rsIdx_1)   = instr(15, 20);
            dcw(rsUse_1) = 1;
            dcw(rsSel_1)   = SRC_A_RS1;
            /////// srcb
            dcw(rsIdx_2)   = instr(20, 25);
            dcw(rsUse_2)   = 0;
            dcw(rsSel_2)   = SRC_B_IMM;

            mWire(aluOpArith, ALU_OP_WIDTH);

            zif(opc == RV32_LOAD){
                dcw(rdUse) = 1;
                dcw(rsEnt) = RS_ENT_LDST;
            } zelif(opc == RV32_STORE){
                dcw(imm_type) = IMM_S;
                dcw(rsEnt)    = RS_ENT_LDST;
                dcw(rsUse_2)  = 1;
            } zelif(opc == RV32_BRANCH){
                zif   (funct3 == RV32_FUNCT3_BEQ ){ dcw(aluOp) = ALU_OP_SEQ; }
                zelif (funct3 == RV32_FUNCT3_BNE ){ dcw(aluOp) = ALU_OP_SNE; }
                zelif (funct3 == RV32_FUNCT3_BLT ){ dcw(aluOp) = ALU_OP_SLT; }
                zelif (funct3 == RV32_FUNCT3_BLTU){ dcw(aluOp) = ALU_OP_SLTU;}
                zelif (funct3 == RV32_FUNCT3_BGE ){ dcw(aluOp) = ALU_OP_SGE; }
                zelif (funct3 == RV32_FUNCT3_BGEU){ dcw(aluOp) = ALU_OP_SGEU;}
                zelse{dcw(illLegal) = 1;}
                dcw(rsEnt)   = RS_ENT_BRANCH;
                dcw(rsUse_2) = 1;
                dcw(rsSel_2) = SRC_B_RS2;

                ///// calculate the next address
                dcw(isBranch) = ~inv;
                if (idx == 1){
                    zif (raw(prCond)){
                        dcw(pred_addr) = raw(npc);
                    }
                }

            } zelif(opc == RV32_JAL){
                dcw(rsEnt)    = RS_ENT_JAL;
                dcw(rsUse_1)  = 0;
                dcw(rsSel_1)  = SRC_A_PC;
                dcw(rsSel_2)  = SRC_B_FOUR;
                dcw(rdUse)    = 1;
                dcw(isBranch) = ~inv;
                if (idx == 1){
                    zif (raw(prCond)){
                        dcw(pred_addr) = raw(npc);
                    }
                }
            } zelif(opc == RV32_JALR){
                dcw(illLegal) = (funct3 != 0);
                dcw(rsEnt)    = RS_ENT_JALR;
                dcw(rsSel_1)  = SRC_A_PC;
                dcw(rsSel_2)  = SRC_B_FOUR;
                dcw(rdUse)    = 1;
                dcw(isBranch) = ~inv;
                if (idx == 1){
                    zif (raw(prCond)){
                        dcw(pred_addr) = raw(npc);
                    }
                }
            } zelif(opc == RV32_OP_IMM){
                dcw(aluOp) = aluOpArith;
                dcw(rdUse) = 1;
            } zelif(opc == RV32_OP){
                dcw(aluOp)   = aluOpArith;
                dcw(rdUse) = 1;
                dcw(rsUse_2) = 1;
                dcw(rsSel_2) = SRC_B_RS2;
                zif( (funct7 == RV32_FUNCT7_MUL_DIV) ){
                    zif ((funct3 == RV32_FUNCT3_MUL) ||
                         (funct3 == RV32_FUNCT3_MULH) ||
                         (funct3 == RV32_FUNCT3_MULHSU) ||
                         (funct3 == RV32_FUNCT3_MULHU)){
                        dcw(rsEnt) = RS_ENT_MUL;
                    }zelse{
                        dcw(rsEnt) = RS_ENT_DIV;
                    }
                }

            } zelif (opc == RV32_AUIPC){
                dcw(imm_type)  = IMM_U;
                dcw(rdUse   )  = 1;
                dcw(rsUse_1 )  = 0;
                dcw(rsSel_1 )  = SRC_A_PC;


            } zelif (opc == RV32_LUI){
                dcw(imm_type) = IMM_U;
                dcw(rdUse   ) = 1;
                dcw(rsUse_1 ) = 0;
                dcw(rsSel_1 ) = SRC_A_ZERO;

            } zelse{
                dcw(illLegal) = 1;
            }

            ///////// alu op
            zif  (funct3 == RV32_FUNCT3_ADD_SUB){
                aluOpArith = ALU_OP_ADD;
                zif (opc == RV32_OP && funct7.sl(5)){
                    aluOpArith = ALU_OP_SUB;
                }
            }
             zelif(funct3 == RV32_FUNCT3_SLL    ){aluOpArith = ALU_OP_SLL;}
             zelif(funct3 == RV32_FUNCT3_SLT    ){aluOpArith = ALU_OP_SLT;}
             zelif(funct3 == RV32_FUNCT3_SLTU   ){aluOpArith = ALU_OP_SLTU;}
             zelif(funct3 == RV32_FUNCT3_XOR    ){aluOpArith = ALU_OP_XOR;}
             zelif(funct3 == RV32_FUNCT3_SRA_SRL && funct7.sl(5)){
                aluOpArith = ALU_OP_SRA;
            }zelif(funct3 == RV32_FUNCT3_SRA_SRL && (~funct7.sl(5))){
                aluOpArith = ALU_OP_SRL;
            }zelif(funct3 == RV32_FUNCT3_OR     ){aluOpArith = ALU_OP_OR;}
             zelif(funct3 == RV32_FUNCT3_AND    ){aluOpArith = ALU_OP_AND;}
             zelse{aluOpArith = ALU_OP_ADD;}

            /////// multiplier

            dcw(md_req_op)          = MD_OP_MUL;
            dcw(md_req_in_signed_1) = 0;
            dcw(md_req_in_signed_2) = 0;
            dcw(md_req_out_sel)     = MD_OUT_LO;

            zif(funct3 == RV32_FUNCT3_MUL){
                // Default values are already set
            }zelif(funct3 == RV32_FUNCT3_MULH){
                dcw(md_req_in_signed_1) = 1;
                dcw(md_req_in_signed_2) = 1;
                dcw(md_req_out_sel) = MD_OUT_HI;
            }zelif(funct3 == RV32_FUNCT3_MULHSU){
                dcw(md_req_in_signed_1) = 1;
                dcw(md_req_out_sel) = MD_OUT_HI;
            }zelif(funct3 == RV32_FUNCT3_MULHU){
                dcw(md_req_out_sel) = MD_OUT_HI;
            }zelif(funct3 == RV32_FUNCT3_DIV){
                dcw(md_req_op) = MD_OP_DIV;
                dcw(md_req_in_signed_1) = 1;
                dcw(md_req_in_signed_2) = 1;
            }zelif(funct3 == RV32_FUNCT3_DIVU){
                dcw(md_req_op) = MD_OP_DIV;
            }zelif(funct3 == RV32_FUNCT3_REM){
                dcw(md_req_op) = MD_OP_REM;
                dcw(md_req_in_signed_1) = 1;
                dcw(md_req_in_signed_2) = 1;
                dcw(md_req_out_sel) = MD_OUT_REM;
           } zelif (funct3 == RV32_FUNCT3_REMU) {
               dcw(md_req_op) = MD_OP_REM;
               dcw(md_req_out_sel) = MD_OUT_REM;
           }
        }


        void flow() override{
            ///// build the decode wire
            decode(1);
            decode(2);
            ///// reference the two lane of decoded register
            RegSlot&  dcd1 = pm.dc.dcd1;
            RegSlot&  dcd2 = pm.dc.dcd2;
            RegSlot&  dcdShared = pm.dc.dcdShared;
            WireSlot& dcw1 = pm.dc.dcw1;
            WireSlot& dcw2 = pm.dc.dcw2;

            ///// check the tag generator is capable to gen the new address tag
            opr& isGenable = tagMgmt.tagGen.isAllGenble(
                dcw1(isBranch), //// isBranch will set when invalid is false and the instruction is jumping instruction
                dcw2(isBranch));

            dbg_isGenable = isGenable;

            pip(pm.dc.sync){                    initProbe(pipProbGrp .decode);
                zyncc(pm.ds.sync, isGenable){   initProbe(zyncProbGrp.decode);
                    ///////// decoded value (except specTag and spec)
                    dcd1 <<= dcw1;
                    dcd2 <<= dcw2;
                    dcdShared(pc)  <<= pm.ft.raw(pc);
                    dcdShared(bhr) <<= pm.ft.raw(bhr);

                    dcdShared(desEqSrc1) <<=
                        ((dcw2(rsIdx_1) == dcw1(rdIdx)) & dcw1(rdUse));
                    dcdShared(desEqSrc2) <<=
                        ((dcw2(rsIdx_2) == dcw1(rdIdx)) & dcw1(rdUse));

                    ///////// generate the tag
                    auto[genTag1, genTag2] =
                    tagMgmt.tagGen.allocate(   //// the tagGen
                    dcw1(isBranch),dcd1(spec), ///// (isBranch) is input
                    dcw2(isBranch),dcd2(spec)  ///// (spec) taggen will update it
                    );
                    ///// assign decoded data
                    dcd1(specTag) <<= genTag1;
                    dcd2(specTag) <<= genTag2;
                    ///// update the mpft
                    tagMgmt.mpft.onAddNew(
                        dcw1(isBranch), genTag1,
                        dcw2(isBranch), genTag2
                    );
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_DECODER_H

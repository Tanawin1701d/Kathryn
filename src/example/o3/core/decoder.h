//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DECODER_H
#define KATHRYN_SRC_EXAMPLE_O3_DECODER_H

#include "kathryn.h"
#include "stageStruct.h"
#include "example/o3/simulation/proberGrp.h"  ///DC


namespace kathryn::o3{

    struct DecMod: Module{  ///MD DECODE
        PipStage& pm;       ///CTRL_HC+DATA_HC DECODE
        TagMgmt& tagMgmt;   ///CTRL_HC DECODE

        mWire(dbg_isGenable, 1); ///DC

        explicit DecMod(                        ///CTRL_HC+DATA_HC DECODE
        PipStage&  pm,                          ///CTRL_HC+DATA_HC DECODE
        TagMgmt&   tagMg) :                     ///CTRL_HC DECODE
            pm(pm),                             ///CTRL_HC+DATA_HC DECODE
            tagMgmt(tagMg){                     ///CTRL_HC DECODE
            pm.dc.dcd1     .makeResetEvent();   ///CTRL_DT+DATA_DT DECODE
            pm.dc.dcd2     .makeResetEvent();   ///CTRL_DT+DATA_DT DECODE
            pm.dc.dcdShared.makeResetEvent();   ///CTRL_DT+DATA_DT DECODE
        }

        ////// dcd = decoded
        void  decode(int idx){ //// idx start at 1 the second is 2   ///HLH DECODE

            bool isFirst = (idx == 1);                                              ///HLH DECODE
            RegSlot&  raw       = pm.ft.raw;                                        ///CTRL_HC+DATA_HC DECODE
            Reg&      instr     = isFirst? pm.ft.raw(inst1)   : pm.ft.raw(inst2);   ///DATA_HC DECODE
            WireSlot& dcwFirst  = pm.dc.dcw1;                                       ///CTRL_HC+DATA_HC DECODE
            WireSlot& dcw       = isFirst? pm.dc.dcw1 : pm.dc.dcw2;                 ///CTRL_HC+DATA_HC DECODE

            mVal(invalid1_dummy, 1, 0);                            ///PARAM DECODE
            opr&      inv   = isFirst? (opr&) invalid1_dummy :     ///CTRL_HC DECODE
                                       (opr&) pm.ft.raw(invalid2); ///CTRL_HC DECODE

            ///////////// src
            Operable& opc     = instr( 0,  7);   ///DATA_CL DECODE
            Operable& funct7  = instr(25, 32);   ///DATA_CL DECODE
            Operable& funct12 = instr(20, 32);   ///DATA_CL DECODE
            Operable& funct3  = instr(12, 15);   ///DATA_CL DECODE

            ///////////// des
            dcw(inst)      = instr;        ///DATA_DT DECODE
            dcw(imm_type)  = IMM_I;        ///DATA_DT DECODE
            dcw(aluOp)     = ALU_OP_ADD;   ///DATA_DT DECODE
            dcw(rsEnt)     = RS_ENT_ALU;   ///DATA_DT DECODE
            dcw(isBranch)  = 0;            ///CTRL_DT DECODE
            dcw(illLegal)  = 0;            ///CTRL_DT DECODE
            ///////////// calculate address and validation
            if (idx == 1){                                                                                         ///HLH DECODE
                dcw(invalid)   = 0; ///// we are sure about first instruction                                      ///CTRL_DT DECODE
                dcw(pred_addr) = raw(pc) + 4; //// if the decoded show it is branch or jump it will be overrided   ///DATA_CL DECODE
            }else{
                /// if the first one is the branch and predict taken we
                dcw(invalid)   = raw(invalid2);                                                                    ///CTRL_DT DECODE
                dcw(pred_addr) = raw(npc);                                                                         ///DATA_DT DECODE
            }


            /////// register translator
            dcw(rdIdx)     = instr(7, 12);   ///DATA_CL DECODE
            dcw(rdUse)     = 0;              ///CTRL_DT DECODE
            /////// srca
            dcw(rsIdx_1)   = instr(15, 20);   ///DATA_CL DECODE
            dcw(rsUse_1) = 1;                 ///CTRL_DT DECODE
            dcw(rsSel_1)   = SRC_A_RS1;       ///DATA_DT DECODE
            /////// srcb
            dcw(rsIdx_2)   = instr(20, 25);   ///DATA_CL DECODE
            dcw(rsUse_2)   = 0;               ///CTRL_DT DECODE
            dcw(rsSel_2)   = SRC_B_IMM;       ///DATA_DT DECODE

            mWire(aluOpArith, ALU_OP_WIDTH);  ///DATA_HWD DECODE


            ztate(opc){                         ///DATA_CL DECODE
                zcase(RV32_LOAD){               ///DATA_CL DECODE
                    dcw(rdUse) = 1;             ///CTRL_DT DECODE
                    dcw(rsEnt) = RS_ENT_LDST;   ///DATA_DT DECODE
                }
                zcase( RV32_STORE){                ///DATA_CL DECODE
                    dcw(imm_type) = IMM_S;         ///DATA_DT DECODE
                    dcw(rsEnt)    = RS_ENT_LDST;   ///DATA_DT DECODE
                    dcw(rsUse_2)  = 1;             ///CTRL_DT DECODE
                }
                zcase(RV32_BRANCH){                                            ///DATA_CL DECODE
                    ztate(funct3){                                             ///DATA_CL DECODE
                        zcase (RV32_FUNCT3_BEQ ){ dcw(aluOp) = ALU_OP_SEQ; }   ///DATA_CL DECODE
                        zcase (RV32_FUNCT3_BNE ){ dcw(aluOp) = ALU_OP_SNE; }   ///DATA_CL DECODE
                        zcase (RV32_FUNCT3_BLT ){ dcw(aluOp) = ALU_OP_SLT; }   ///DATA_CL DECODE
                        zcase (RV32_FUNCT3_BLTU){ dcw(aluOp) = ALU_OP_SLTU;}   ///DATA_CL DECODE
                        zcase (RV32_FUNCT3_BGE ){ dcw(aluOp) = ALU_OP_SGE; }   ///DATA_CL DECODE
                        zcase (RV32_FUNCT3_BGEU){ dcw(aluOp) = ALU_OP_SGEU;}   ///DATA_CL DECODE
                        zcasedef{dcw(illLegal) = 1;}                           ///CTRL_CL DECODE
                    }

                    dcw(rsEnt)   = RS_ENT_BRANCH;   ///DATA_DT DECODE
                    dcw(rsUse_2) = 1;               ///CTRL_DT DECODE
                    dcw(rsSel_2) = SRC_B_RS2;       ///DATA_DT DECODE

                    ///// calculate the next address
                    dcw(isBranch) = ~inv;   ///CTRL_CL DECODE
                }
                zcase(RV32_JAL){                  ///DATA_CL DECODE
                    dcw(rsEnt)    = RS_ENT_JAL;   ///DATA_DT DECODE
                    dcw(rsUse_1)  = 0;            ///CTRL_DT DECODE
                    dcw(rsSel_1)  = SRC_A_PC;     ///DATA_DT DECODE
                    dcw(rsSel_2)  = SRC_B_FOUR;   ///DATA_DT DECODE
                    dcw(rdUse)    = 1;            ///CTRL_DT DECODE
                    dcw(isBranch) = ~inv;         ///CTRL_CL DECODE
                }
                zcase(RV32_JALR){                    ///DATA_CL DECODE
                    dcw(illLegal) = (funct3 != 0);   ///CTRL_CL DECODE
                    dcw(rsEnt)    = RS_ENT_JALR;     ///DATA_DT DECODE
                    dcw(rsSel_1)  = SRC_A_PC;        ///DATA_DT DECODE
                    dcw(rsSel_2)  = SRC_B_FOUR;      ///DATA_DT DECODE
                    dcw(rdUse)    = 1;               ///CTRL_DT DECODE
                    dcw(isBranch) = ~inv;            ///CTRL_CL DECODE
                }
                zcase(RV32_OP_IMM){            ///DATA_CL DECODE
                    dcw(aluOp) = aluOpArith;   ///DATA_DT DECODE
                    dcw(rdUse) = 1;            ///CTRL_DT DECODE
                }
                zcase(RV32_OP){                                ///DATA_CL DECODE
                    dcw(aluOp)   = aluOpArith;                 ///DATA_DT DECODE
                    dcw(rdUse) = 1;                            ///CTRL_DT DECODE
                    dcw(rsUse_2) = 1;                          ///CTRL_DT DECODE
                    dcw(rsSel_2) = SRC_B_RS2;                  ///DATA_DT DECODE
                    zif( (funct7 == RV32_FUNCT7_MUL_DIV) ){    ///DATA_CL DECODE
                        zif ((funct3 == RV32_FUNCT3_MUL) ||    ///DATA_CL DECODE
                             (funct3 == RV32_FUNCT3_MULH) ||   ///DATA_CL DECODE
                             (funct3 == RV32_FUNCT3_MULHSU) || ///DATA_CL DECODE
                             (funct3 == RV32_FUNCT3_MULHU)){   ///DATA_CL DECODE
                            dcw(rsEnt) = RS_ENT_MUL;           ///DATA_DT DECODE
                        }zelse{
                            dcw(rsEnt) = RS_ENT_DIV;   ///DATA_DT DECODE
                        }
                    }
                }
                zcase(RV32_AUIPC){               ///DATA_CL DECODE
                    dcw(imm_type)  = IMM_U;      ///DATA_DT DECODE
                    dcw(rdUse   )  = 1;          ///CTRL_DT DECODE
                    dcw(rsUse_1 )  = 0;          ///CTRL_DT DECODE
                    dcw(rsSel_1 )  = SRC_A_PC;   ///DATA_DT DECODE
                }
                zcase(RV32_LUI){                  ///DATA_CL DECODE
                    dcw(imm_type) = IMM_U;        ///DATA_DT DECODE
                    dcw(rdUse   ) = 1;            ///CTRL_DT DECODE
                    dcw(rsUse_1 ) = 0;            ///CTRL_DT DECODE
                    dcw(rsSel_1 ) = SRC_A_ZERO;   ///DATA_DT DECODE
                }
                zcasedef{                ///DATA_CL DECODE
                    dcw(illLegal) = 1;   ///CTRL_DT DECODE
                }
            }

            ztate(funct3){                                  ///DATA_CL DECODE
                zcase(RV32_FUNCT3_ADD_SUB){                 ///DATA_CL DECODE
                    aluOpArith = ALU_OP_ADD;                ///DATA_DT DECODE
                    zif (opc == RV32_OP && funct7.sl(5)){   ///DATA_CL DECODE
                        aluOpArith = ALU_OP_SUB;            ///DATA_DT DECODE
                    }
                }
                zcase(RV32_FUNCT3_SLL    ){aluOpArith = ALU_OP_SLL;}   ///DATA_CL DECODE
                zcase(RV32_FUNCT3_SLT    ){aluOpArith = ALU_OP_SLT;}   ///DATA_CL DECODE
                zcase(RV32_FUNCT3_SLTU   ){aluOpArith = ALU_OP_SLTU;}  ///DATA_CL DECODE
                zcase(RV32_FUNCT3_XOR    ){aluOpArith = ALU_OP_XOR;}   ///DATA_CL DECODE
                zcase(RV32_FUNCT3_SRA_SRL){                            ///DATA_CL DECODE
                    zif(funct7.sl(5)){ aluOpArith = ALU_OP_SRA;}       ///DATA_CL DECODE
                    zelse            { aluOpArith = ALU_OP_SRL;}       ///DATA_CL DECODE
                }
                zcase(RV32_FUNCT3_OR     ){aluOpArith = ALU_OP_OR;}    ///DATA_CL DECODE
                zcase(RV32_FUNCT3_AND    ){aluOpArith = ALU_OP_AND;}   ///DATA_CL DECODE
                zcasedef{aluOpArith = ALU_OP_ADD;}                     ///DATA_CL DECODE
            }

            /////// multiplier

            dcw(md_req_op)          = MD_OP_MUL;   ///DATA_DT DECODE
            dcw(md_req_in_signed_1) = 0;           ///DATA_DT DECODE
            dcw(md_req_in_signed_2) = 0;           ///DATA_DT DECODE
            dcw(md_req_out_sel)     = MD_OUT_LO;   ///DATA_DT DECODE


            ztate(funct3){   ///DATA_CL DECODE
                // zcase(RV32_FUNCT3_MUL){
                //     // Default values are already set
                // }
                zcase(RV32_FUNCT3_MULH){                ///DATA_CL DECODE
                    dcw(md_req_in_signed_1) = 1;        ///DATA_DT DECODE
                    dcw(md_req_in_signed_2) = 1;        ///DATA_DT DECODE
                    dcw(md_req_out_sel) = MD_OUT_HI;    ///DATA_DT DECODE
                }zcase(RV32_FUNCT3_MULHSU){             ///DATA_CL DECODE
                    dcw(md_req_in_signed_1) = 1;        ///DATA_DT DECODE
                    dcw(md_req_out_sel) = MD_OUT_HI;    ///DATA_DT DECODE
                }zcase(RV32_FUNCT3_MULHU){              ///DATA_CL DECODE
                    dcw(md_req_out_sel) = MD_OUT_HI;    ///DATA_DT DECODE
                }zcase(RV32_FUNCT3_DIV){                ///DATA_CL DECODE
                    dcw(md_req_op) = MD_OP_DIV;         ///DATA_DT DECODE
                    dcw(md_req_in_signed_1) = 1;        ///DATA_DT DECODE
                    dcw(md_req_in_signed_2) = 1;        ///DATA_DT DECODE
                }zcase(RV32_FUNCT3_DIVU){               ///DATA_CL DECODE
                    dcw(md_req_op) = MD_OP_DIV;         ///DATA_DT DECODE
                }zcase(RV32_FUNCT3_REM){                ///DATA_CL DECODE
                    dcw(md_req_op) = MD_OP_REM;         ///DATA_DT DECODE
                    dcw(md_req_in_signed_1) = 1;        ///DATA_DT DECODE
                    dcw(md_req_in_signed_2) = 1;        ///DATA_DT DECODE
                    dcw(md_req_out_sel) = MD_OUT_REM;   ///DATA_DT DECODE
                }zcase(RV32_FUNCT3_REMU) {              ///DATA_CL DECODE
                    dcw(md_req_op) = MD_OP_REM;         ///DATA_DT DECODE
                    dcw(md_req_out_sel) = MD_OUT_REM;   ///DATA_DT DECODE
                }
            }
        }


        void flow() override{                        ///HLH DECODE
            ///// build the decode wire
            decode(1);                               ///HLH DECODE
            decode(2);                               ///HLH DECODE
            ///// reference the two lane of decoded register
            RegSlot&  dcd1 = pm.dc.dcd1;             ///CTRL_HC+DATA_HC DECODE
            RegSlot&  dcd2 = pm.dc.dcd2;             ///CTRL_HC+DATA_HC DECODE
            RegSlot&  dcdShared = pm.dc.dcdShared;   ///CTRL_HC+DATA_HC DECODE
            WireSlot& dcw1 = pm.dc.dcw1;             ///CTRL_HC+DATA_HC DECODE
            WireSlot& dcw2 = pm.dc.dcw2;             ///CTRL_HC+DATA_HC DECODE

            ///// check the tag generator is capable to gen the new address tag
            opr& isGenable = tagMgmt.tagGen.isAllGenble(                                                                  ///CTRL_CL DECODE
                dcw1(isBranch), //// isBranch will set when invalid is false and the instruction is jumping instruction   ///CTRL_HC DECODE
                dcw2(isBranch));                                                                                          ///CTRL_HC DECODE

            dbg_isGenable = isGenable; ///DC

            pip(pm.dc.sync){                    initProbe(pipProbGrp .decode);   ///CTRL_HWD+CTRL_CL DECODE
                zyncc(pm.sync_dp, isGenable){   initProbe(zyncProbGrp.decode);   ///CTRL_HWD+CTRL_CL DECODE
                    ///////// decoded value (except specTag and spec)
                    dcd1 <<= dcw1;                      ///CTRL_DT+DATA_DT DECODE
                    dcd2 <<= dcw2;                      ///CTRL_DT+DATA_DT DECODE
                    dcdShared(pc)  <<= pm.ft.raw(pc);   ///DATA_DT DECODE

                    dcdShared(desEqSrc1) <<=                            ///CTRL_CL DECODE
                        ((dcw2(rsIdx_1) == dcw1(rdIdx)) & dcw1(rdUse)); ///CTRL_CL DECODE
                    dcdShared(desEqSrc2) <<=                            ///CTRL_CL DECODE
                        ((dcw2(rsIdx_2) == dcw1(rdIdx)) & dcw1(rdUse)); ///CTRL_CL DECODE

                    ///////// generate the tag
                    auto[genTag1, genTag2] =   ///HLH DECODE
                    tagMgmt.tagGen.allocate(   //// the tagGen                    ///CTRL_HC DECODE
                    dcw1(isBranch),dcd1(spec), ///// (isBranch) is input          ///CTRL_HC DECODE
                    dcw2(isBranch),dcd2(spec)  ///// (spec) taggen will update it ///CTRL_HC DECODE
                    );
                    ///// assign decoded data
                    dcd1(specTag) <<= genTag1;   ///CTRL_DT DECODE
                    dcd2(specTag) <<= genTag2;   ///CTRL_DT DECODE
                    ///// update the mpft
                    tagMgmt.mpft.onAddNew(   ///CTRL_HC DECODE
                        dcw1(isBranch), genTag1, ///CTRL_HC DECODE
                        dcw2(isBranch), genTag2 ///CTRL_HC DECODE
                    );
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_DECODER_H

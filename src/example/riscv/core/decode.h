//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_DECODE_H
#define KATHRYN_DECODE_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "example/numberic/extend.h"

namespace kathryn{

    namespace riscv{

        class Decode {

            Slice OP_ALL{0,7};
            Slice OP_H  { 5, 7};
            Slice OP_L  { 2, 5};
            Slice IDX_RD{ 7,12};
            Slice IDX_R1{15,20};
            Slice IDX_R2{20,25};
            Slice FUNCT3{12,15};
            Slice FUNCT7{25,32};

            Slice IMM_I_0_12 {20, 32};

            Slice IMM_S_5_12 {25, 32};
            Slice IMM_S_0_5  {7, 12};

            Slice IMM_B_12   {31, 32};
            Slice IMM_B_5_11 {25, 31};
            Slice IMM_B_11   {7, 8};
            Slice IMM_B_1_5  {8, 12};

            Slice IMM_U_12_32{12, 32};

            Slice IMM_J_20   {31, 32};
            Slice IMM_J_1_11 {21,31};
            Slice IMM_J_11   {20};
            Slice IMM_J_12_20{12,20};

        public:
            UOp decInstr;
            makeWire(invalidHighDec, 1);
            makeWire(invalidLowDec,  1);



            void flow(Operable& rst, FETCH_DATA& fetchData) {

                pipBlk {
                    intrReset(rst);
                    par {

                        /////// deal with pc
                        decInstr.pc     <<= fetchData.fetch_pc;
                        decInstr.nextPc <<= fetchData.fetch_nextpc;
                        auto& op= fetchData.fetch_instr(OP_ALL);

                        zif( op(OP_H) == 0b00){
                            //////// load  immop aulpc ////////////////////////////////////////////////
                            zif (op(OP_L) == 0b000){
                                doLoadStoreDecode(true, fetchData.fetch_instr);
                            }///zelif(op(2,5) == 0b11){//doMiscDecode(fetchBlk.fetch_instr);///}
                            zelif(op(OP_L) == 0b100){
                                doOpDecode(fetchData.fetch_instr, false); ///// imm
                            }zelse{ ///////101
                                doAulPcDecode(fetchData.fetch_instr);
                            }
                            ///////////////////////////////////////////////////////////////////////////
                        }zelif(op(OP_H) == 0b01){
                            //////////// store op luidcode////////////////////////////////////////////
                            zif (op(OP_L) == 0b000){
                                doLoadStoreDecode(false, fetchData.fetch_instr);
                            }zelif(op(OP_L) == 0b100){
                                doOpDecode(fetchData.fetch_instr, true);
                            }zelse{ //(op(2,5) == 0b101) {
                                doLuiDecode(fetchData.fetch_instr);
                            }
                            ////////////////////////////////////////////////////////////////////////
                        }zelif(op(OP_H) == 0b11){ ////// 11   we dont support 10
                            ////////// branch jump with reg //////////////////////////
                            zif (op(OP_L) == 0b000){
                                doBranchDecode(fetchData.fetch_instr);
                            }zelif(op(OP_L) == 0b001){
                                doJalRDecode(fetchData.fetch_instr);
                            }zelse{////(op(2,5) == 0b011){ this is 11
                                doJalDecode(fetchData.fetch_instr);
                            }//}zelse{
//                                doSystemDecode(fetchBlk.fetch_instr);
//                            }
                            ///////////////////////////////////////////////////////////
                        }zelse{
                            /////// 11
                            invalidHighDec = 1;
                        };

                    }
                }

            }

            /** 00**/

            void doLoadStoreDecode(bool isLoad, Reg& instr){

                Slice sizeBit = {12, 14};
                int extendModeBit = 14;

                decInstr.regData[RS_1].setFromRegFile(instr(IDX_R1));
                if (isLoad){
                    decInstr.regData[RS_2].reset();
                    decInstr.regData[RS_3].setFromImm(getExtendExpr(instr(IMM_I_0_12), XLEN, false));
                    decInstr.regData[RS_des].setFromRegFile(instr(IDX_RD));
                }else{
                    decInstr.regData[RS_2].setFromRegFile(instr(IDX_R2));
                    decInstr.regData[RS_3].setFromImm(
                        getExtendExpr(g(instr(IMM_S_5_12), instr(IMM_S_0_5)), XLEN, false)
                            );
                    decInstr.regData[RS_des].reset();

                }

                decInstr.opLs.set(isLoad,instr(sizeBit), instr(extendModeBit));
                decInstr.opAlu     .reset();
                decInstr.opCtrlFlow.reset();
                decInstr.opLdPc    .reset();
            }

            void doOpDecode(Reg& instr, bool isUsedR2){ //// else is imm

                /** deal with reg */
                decInstr.regData[RS_1].setFromRegFile(instr(IDX_R1));
                if (isUsedR2){
                    decInstr.regData[RS_2].setFromRegFile(instr(IDX_R2));
                }else{
                    /** for shift user only lower 5 bit*/
                    decInstr.regData[RS_2].setFromImm(getExtendExpr(instr(IMM_I_0_12), XLEN, false));
                }
                decInstr.regData[RS_3].reset();
                decInstr.regData[RS_des].setFromRegFile(instr(IDX_RD));

                /**reset other instruction*/
                decInstr.opLs      .reset();
                decInstr.opCtrlFlow.reset();
                decInstr.opLdPc    .reset();
                /**set current instruction*/
                decInstr.opAlu.isUopUse                <<= 1;
                if (isUsedR2) {
                    decInstr.opAlu.isAdd               <<= (instr(FUNCT3) == 0b000) & (instr(30) == 0);
                    decInstr.opAlu.isSub               <<= (instr(FUNCT3) == 0b000) & (instr(30) == 1);
                }else{
                    decInstr.opAlu.isAdd <<= (instr(FUNCT3) == 0b000);
                    decInstr.opAlu.isSub <<= 0;
                }
                decInstr.opAlu.isXor               <<= (instr(FUNCT3) == 0b100);
                decInstr.opAlu.isOr                <<= (instr(FUNCT3) == 0b110);
                decInstr.opAlu.isAnd               <<= (instr(FUNCT3) == 0b111);
                decInstr.opAlu.isCmpLessThanSign   <<= (instr(FUNCT3) == 0b010);
                decInstr.opAlu.isCmpLessThanUSign  <<= (instr(FUNCT3) == 0b011);
                decInstr.opAlu.isShiftLeftLogical  <<= (instr(FUNCT3) == 0b001);
                decInstr.opAlu.isShiftRightLogical <<= (instr(FUNCT3) == 0b101) & (instr(30) == 0);
                decInstr.opAlu.isShiftRightArith   <<= (instr(FUNCT3) == 0b101) & (instr(30) == 1);


            }

            void doAulPcDecode(Reg& instr, bool needPc = false){
                decInstr.regData[RS_1].reset();
                makeVal(fixdown, 12, 0);
                decInstr.regData[RS_2]  .setFromImm(g(instr(IMM_U_12_32),  fixdown));
                decInstr.regData[RS_3]  .reset();
                decInstr.regData[RS_des].setFromRegFile(instr(IDX_RD));

                decInstr.opLs      .reset();
                decInstr.opAlu     .reset();
                decInstr.opCtrlFlow.reset();
                decInstr.opLdPc.isUopUse <<= 1;
                decInstr.opLdPc.needPc   <<= needPc;

            }

            void doLuiDecode(Reg& instr){
                doAulPcDecode(instr, true);
            }

            /***
             *
             * branch control
             *
             * */

            void doBranchDecode(Reg& instr){
                decInstr.regData[RS_1].setFromRegFile(instr(IDX_R1));
                decInstr.regData[RS_2].setFromRegFile(instr(IDX_R2));

                makeVal(lastBit, 1, 0);
                nest& rawNest = g(instr(IMM_B_12),instr(IMM_B_11),instr(IMM_B_5_11),instr(IMM_B_1_5), lastBit);

                decInstr.regData[RS_3].setFromImm(getExtendExpr(rawNest, XLEN, false));
                decInstr.regData[RS_des].setFromRegFile(instr(IDX_RD));

                auto funct3 = instr(FUNCT3);

                decInstr.opLs.reset();
                decInstr.opAlu.reset();
                decInstr.opCtrlFlow.isUopUse <<= 1;
                decInstr.opCtrlFlow.extendMode <<= instr(13);
                decInstr.opCtrlFlow.isJalR     <<= 0b0;
                decInstr.opCtrlFlow.isJal      <<= 0b0;
                decInstr.opCtrlFlow.isEq       <<= (funct3 == 0b0);
                decInstr.opCtrlFlow.isNEq      <<= (funct3 == 0b001);
                decInstr.opCtrlFlow.isLt       <<= (funct3 == 0b100) | (funct3 == 0b110);
                decInstr.opCtrlFlow.isGe       <<= (funct3 == 0b101) | (funct3 == 0b111);


                decInstr.opLdPc.reset();
            }

            /**
             *
             * jump control
             *
             * */

            void doJalRDecode(Reg& instr){

                doJalDecode(instr, true);

            }

            void doJalDecode(Reg& instr, bool reqR1 = false){

                if (reqR1){
                    decInstr.regData[RS_1].setFromRegFile(instr(IDX_R1));
                    decInstr.regData[RS_2].setFromImm(getExtendExpr(instr(IMM_I_0_12), XLEN, false));
                }else{
                    decInstr.regData[RS_1].setZero();
                    makeVal(lastBit, 1, 0);
                    nest& rawNest = g(instr(IMM_J_20),instr(IMM_J_12_20),instr(IMM_J_11),instr(IMM_J_1_11), lastBit);
                    decInstr.regData[RS_2].setFromImm(getExtendExpr(rawNest, XLEN, false));
                }
                decInstr.regData[RS_3]  .reset();
                decInstr.regData[RS_des].setFromRegFile(instr(IDX_RD));

                decInstr.opLs.reset();
                decInstr.opAlu.reset();
                decInstr.opLdPc.reset();

                decInstr.opCtrlFlow.isUopUse   <<= 1;
                decInstr.opCtrlFlow.extendMode <<= 0;
                decInstr.opCtrlFlow.isJalR     <<= reqR1;
                decInstr.opCtrlFlow.isJal      <<= !reqR1;
                decInstr.opCtrlFlow.isEq       <<= 0;
                decInstr.opCtrlFlow.isNEq      <<= 0;
                decInstr.opCtrlFlow.isLt       <<= 0;
                decInstr.opCtrlFlow.isGe       <<= 0;

            }

        };


    }

}
#endif //KATHRYN_DECODE_H


//        enum RISCV_OP{
//            /*** mem memory related opcode */
//            RS_LOAD    = 0b00'000,
//            RS_STORE   = 0b01'000,
//            RS_MISCMEM = 0b00'011,
//            /***  branch instruction*/
//            RS_BRANCH  = 0b11'000,
//            RS_JALR    = 0b11'001,
//            RS_JAL     = 0b11'011,
//            /***  arithmthic instruction*/
//            RS_OP_IMM  = 0b00'100,
//            RS_OP      = 0b01'100,
//
//            RS_AUIPC   = 0b00'101,
//            RS_LUI     = 0b01'101,
//            /**  system instruction*/
//            RS_SYSTEM   = 0b11'100,
//        };


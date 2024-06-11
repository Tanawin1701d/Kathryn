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

            Slice OP_ALL{ 0, 7};
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
            Slice IMM_J_11   {20, 21};
            Slice IMM_J_12_20{12,20};

        public:
            mWire(parCheck,       1);
            mWire(invalidHighDec, 1);
            mWire(invalidLowDec,  1);
            FlowBlockBase* decodeBlk = nullptr;

            void flow(Operable& rst, FETCH_DATA& fetchData, UOp& _decUop) {

                pipBlk { intrReset(rst); intrStart(rst); exposeBlk(decodeBlk)
                    cdowhile(!nextPipReadySig) {
                        ///parCheck = 1;
                        par {
                            zif(nextPipReadySig) {
                                /////// deal with pc
                                _decUop.pc     <<= fetchData.fetch_pc;
                                _decUop.nextPc <<= fetchData.fetch_nextpc;
                                auto& op = fetchData.fetch_instr(OP_ALL);
                                zif (op(0,2) == 0b11) {
                                    zif(op(OP_H) == 0b00) {
                                        //////// load  immop aulpc ////////////////////////////////////////////////
                                        zif  (op(OP_L) == 0b000) {
                                            doLoadStoreDecode(fetchData.fetch_instr, _decUop, true);
                                        }///zelif(op(2,5) == 0b11){//doMiscDecode(fetchBlk.fetch_instr);///}
                                        zelif(op(OP_L) == 0b100) { doOpDecode(fetchData.fetch_instr, _decUop, false); }
                                        zelse { doAulPcDecode(fetchData.fetch_instr, _decUop);/*101*/         }

                                    }zelif(op(OP_H) == 0b01) {
                                        //////////// store op luidcode////////////////////////////////////////////
                                        zif  (op(OP_L) == 0b000) {
                                            doLoadStoreDecode(fetchData.fetch_instr, _decUop, false);
                                        }zelif(op(OP_L) == 0b100) { doOpDecode(fetchData.fetch_instr, _decUop, true); }
                                        zelse { doLuiDecode(fetchData.fetch_instr, _decUop); }   //(op(2,5) == 0b101) {
                                        ////////////////////////////////////////////////////////////////////////
                                    }zelif(op(OP_H) == 0b11) { ////// 11   we dont support 10
                                        ////////// branch jump with reg //////////////////////////
                                        zif  (op(OP_L) == 0b000) { doBranchDecode(fetchData.fetch_instr, _decUop); }
                                        zelif(op(OP_L) == 0b001) { doJalRDecode(fetchData.fetch_instr, _decUop); }
                                        zelse {
                                            doJalDecode(fetchData.fetch_instr, _decUop);
                                        }/*(op(2,5) == 0b011){ this is 11*///}zelse{doSystemDecode(fetchBlk.fetch_instr);
                                    }zelse {
                                        invalidHighDec = 1;
                                        _decUop.reset();
                                    }
                                }zelse{
                                    _decUop.reset();
                                };
                            }

                        }
                    }
                }

            }

            /** 00**/

            void doLoadStoreDecode(Reg& instr, UOp& _decUop, bool isLoad){

                Slice sizeBit = {12, 14};
                int extendModeBit = 14;

                _decUop.regData[RS_1].setFromRegFile(instr(IDX_R1));
                if (isLoad){
                    _decUop.regData[RS_2].reset();
                    _decUop.regData[RS_3].setFromImm(getExtendExpr(instr(IMM_I_0_12), XLEN, false));
                    _decUop.regData[RS_des].setFromRegFile(instr(IDX_RD));
                }else{
                    _decUop.regData[RS_2].setFromRegFile(instr(IDX_R2));
                    _decUop.regData[RS_3].setFromImm(
                        getExtendExpr(g(instr(IMM_S_5_12), instr(IMM_S_0_5)), XLEN, false)
                            );
                    _decUop.regData[RS_des].reset();

                }

                _decUop.opLs      .set(isLoad, instr(sizeBit), instr(extendModeBit));
                _decUop.opAlu     .reset();
                _decUop.opCtrlFlow.reset();
                _decUop.opLdPc    .reset();
            }

            void doOpDecode(Reg& instr, UOp& _decUop, bool isUsedR2){ //// else is imm

                /** deal with reg */
                _decUop.regData[RS_1].setFromRegFile(instr(IDX_R1));
                if (isUsedR2){
                    _decUop.regData[RS_2].setFromRegFile(instr(IDX_R2));
                }else{
                    /** for shift user only lower 5 bit*/
                    _decUop.regData[RS_2].setFromImm(getExtendExpr(instr(IMM_I_0_12), XLEN, false));
//                    mVal(testdd, 32, 7);
//                    _decUop.regData[RS_2].setFromImm(testdd);
                }
                _decUop.regData[RS_3].reset();
                _decUop.regData[RS_des].setFromRegFile(instr(IDX_RD));

                /**reset other instruction*/
                _decUop.opLs      .reset();
                _decUop.opCtrlFlow.reset();
                _decUop.opLdPc    .reset();
                /**set current instruction*/
                _decUop.opAlu.isUopUse                <<= 1;
                if (isUsedR2) {
                    _decUop.opAlu.isAdd               <<= (instr(FUNCT3) == 0b000) & (instr(30) == 0);
                    _decUop.opAlu.isSub               <<= (instr(FUNCT3) == 0b000) & (instr(30) == 1);
                }else{
                    _decUop.opAlu.isAdd <<= (instr(FUNCT3) == 0b000);
                    _decUop.opAlu.isSub <<= 0;
                }
                _decUop.opAlu.isXor               <<= (instr(FUNCT3) == 0b100);
                _decUop.opAlu.isOr                <<= (instr(FUNCT3) == 0b110);
                _decUop.opAlu.isAnd               <<= (instr(FUNCT3) == 0b111);
                _decUop.opAlu.isCmpLessThanSign   <<= (instr(FUNCT3) == 0b010);
                _decUop.opAlu.isCmpLessThanUSign  <<= (instr(FUNCT3) == 0b011);
                _decUop.opAlu.isShiftLeftLogical  <<= (instr(FUNCT3) == 0b001);
                _decUop.opAlu.isShiftRightLogical <<= (instr(FUNCT3) == 0b101) & (instr(30) == 0);
                _decUop.opAlu.isShiftRightArith   <<= (instr(FUNCT3) == 0b101) & (instr(30) == 1);


            }

            void doAulPcDecode(Reg& instr, UOp& _decUop,bool needPc = true){
                _decUop.regData[RS_1].reset();
                mVal(fixdown, 12, 0);
                _decUop.regData[RS_2]  .setFromImm(g(instr(IMM_U_12_32), fixdown));
                _decUop.regData[RS_3]  .reset();
                _decUop.regData[RS_des].setFromRegFile(instr(IDX_RD));

                _decUop.opLs      .reset();
                _decUop.opAlu     .reset();
                _decUop.opCtrlFlow.reset();
                _decUop.opLdPc.isUopUse <<= 1;
                _decUop.opLdPc.needPc   <<= needPc;

            }

            void doLuiDecode(Reg& instr, UOp& _decUop){
                doAulPcDecode(instr, _decUop, false);
            }

            /***
             *
             * branch control
             *
             * */

            void doBranchDecode(Reg& instr, UOp& _decUop){
                _decUop.regData[RS_1].setFromRegFile(instr(IDX_R1));
                _decUop.regData[RS_2].setFromRegFile(instr(IDX_R2));

                mVal(lastBit, 1, 0);
                nest& rawNest = g(instr(IMM_B_12),instr(IMM_B_11),instr(IMM_B_5_11),instr(IMM_B_1_5), lastBit);

                _decUop.regData[RS_3].setFromImm(getExtendExpr(rawNest, XLEN, false));
                _decUop.regData[RS_des].setFromRegFile(instr(IDX_RD));

                auto& funct3 = instr(FUNCT3);

                _decUop.opLs.reset();
                _decUop.opAlu.reset();
                _decUop.opCtrlFlow.isUopUse <<= 1;
                _decUop.opCtrlFlow.extendMode <<= instr(13);
                _decUop.opCtrlFlow.isJalR     <<= 0b0;
                _decUop.opCtrlFlow.isJal      <<= 0b0;
                _decUop.opCtrlFlow.isEq       <<= (funct3 == 0b000);
                _decUop.opCtrlFlow.isNEq      <<= (funct3 == 0b001);
                _decUop.opCtrlFlow.isLt       <<= (funct3 == 0b100) | (funct3 == 0b110);
                _decUop.opCtrlFlow.isGe       <<= (funct3 == 0b101) | (funct3 == 0b111);


                _decUop.opLdPc.reset();
            }

            /**
             *
             * jump control
             *
             * */

            void doJalRDecode(Reg& instr, UOp& _decUop){
                doJalDecode(instr, _decUop, true);
            }

            void doJalDecode(Reg& instr, UOp& _decUop,bool reqR1 = false){

                if (reqR1){
                    _decUop.regData[RS_1].setFromRegFile(instr(IDX_R1));
                    _decUop.regData[RS_2].setFromImm(getExtendExpr(instr(IMM_I_0_12), XLEN, false));
                }else{
                    _decUop.regData[RS_1].setZero();
                    mVal(lastBit, 1, 0);
                    nest& rawNest = g(instr(IMM_J_20),instr(IMM_J_12_20),instr(IMM_J_11),instr(IMM_J_1_11), lastBit);
                    _decUop.regData[RS_2].setFromImm(getExtendExpr(rawNest, XLEN, false));
                }
                _decUop.regData[RS_3]  .reset();
                _decUop.regData[RS_des].setFromRegFile(instr(IDX_RD));

                _decUop.opLs.reset();
                _decUop.opAlu.reset();
                _decUop.opLdPc.reset();

                _decUop.opCtrlFlow.isUopUse   <<= 1;
                _decUop.opCtrlFlow.extendMode <<= 0;
                _decUop.opCtrlFlow.isJalR     <<= reqR1;
                _decUop.opCtrlFlow.isJal      <<= !reqR1;
                _decUop.opCtrlFlow.isEq       <<= 0;
                _decUop.opCtrlFlow.isNEq      <<= 0;
                _decUop.opCtrlFlow.isLt       <<= 0;
                _decUop.opCtrlFlow.isGe       <<= 0;

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


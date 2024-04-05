//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_DECODE_H
#define KATHRYN_DECODE_H

#include "fetch.h"
#include "example/riscv/element.h"

namespace kathryn{


    namespace riscv {

        enum RISCV_OP{
            /*** mem memory related opcode */
            RS_LOAD    = 0b00'000,
            RS_STORE   = 0b01'000,
            RS_MISCMEM = 0b00'011,
            /***  branch instruction*/
            RS_BRANCH  = 0b11'000,
            RS_JALR    = 0b11'001,
            RS_JAL     = 0b11'011,
            /***  arithmthic instruction*/
            RS_OP_IMM  = 0b00'100,
            RS_OP      = 0b01'100,

            RS_AUIPC   = 0b00'101,
            RS_LUI     = 0b01'101,
            /**  system instruction*/
            RS_SYSTEM   = 0b11'100,
        };


        class Decode {

            UOp decInstr;

        public:

            void flow(Fetch &fetchBlk) {

                pipBlk {
                    par {

                        /////// deal with pc
                        decInstr.pc <<= fetchBlk.fetch_pc;
                        decInstr.nextPc <<= fetchBlk.fetch_nextpc;
                        auto op = fetchBlk.fetch_instr(0, 7);

                        zif( op(5, 7) == 0b0){
                            zif (op(2, 5) == 0){
                                doLoadStoreDecode(true, fetchBlk.fetch_instr);
                            }///zelif(op(2,5) == 0b11){
                                //doMiscDecode(fetchBlk.fetch_instr);
                            ///}
                            zelif(op(2,5) == 0b100){
                                doOpImmDecode(fetchBlk.fetch_instr);
                            }zelse{
                                doAulPcDecode(fetchBlk.fetch_instr);
                            }
                        }zelif(op(5, 7) == 0b01){
                            zif (op(2, 5) == 0){
                                doLoadStoreDecode(false, fetchBlk.fetch_instr);
                            }zelif(op(2,5) == 0b100){
                                doOpDecode(fetchBlk.fetch_instr);
                            }zelse{ //(op(2,5) == 0b101) {
                                doLuiDecode(fetchBlk.fetch_instr);
                            }
                        }zelse{ ////// 11
                            zif (op(2, 5) == 0){
                                doBranchDecode(fetchBlk.fetch_instr);
                            }zelif(op(2,5) == 0b1){
                                doJalRDecode(fetchBlk.fetch_instr);
                            }zelse {////(op(2,5) == 0b011){
                                doJalDecode(fetchBlk.fetch_instr);
                            }//}zelse{
//                                doSystemDecode(fetchBlk.fetch_instr);
//                            }
                        }



                    }
                }

            }

            /** 00**/

            void doLoadStoreDecode(bool isLoad, Reg& instr){
                if (isLoad){
                    decInstr.opLs.isMemLoad  <<= 1;
                    decInstr.opLs.isMemStore <<= 0;
                    decInstr.regData[RS_1].setFromRegFile(instr(15, 20));
                    zif(instr(14)){
                        decInstr.regData[RS_2].setFromImm(); ///// set from sign extend mode
                    }zelse{
                        decInstr.regData[RS_2].setFromImm();
                    };
                    decInstr.regData[RS_des].setFromRegFile(instr(7, 12));


                }else{
                    decInstr.opLs.isMemLoad  <<= 0;
                    decInstr.opLs.isMemStore <<= 1;
                    decInstr.regData[RS_1].setFromRegFile(instr(15, 20));
                    decInstr.regData[RS_2].setFromImm();
                    decInstr.regData[RS_des].setFromImm();

                }
                decInstr.regData[RS_3].reset();
                decInstr.opAlu        .reset();
                decInstr.opBranch     .reset();
                decInstr.opLdPc       .reset();

                decInstr.opLs.isUopUse  <<= 1;
                decInstr.opLs.size         <<= instr(12, 14);
                decInstr.opLs.extendMode   <<= instr(14);
            }
//
//            void doMiscDecode(Reg& instr){
//
//
//
//            }

            void doOpImmDecode(Reg& instr){

                decInstr.regData[RS_1].setFromRegFile(instr(15, 20));
                decInstr.regData[RS_2].setFromImm();
                decInstr.regData[RS_3].reset();
                decInstr.regData[RS_des].setFromRegFile(instr(7, 12));

                decInstr.opLs.reset();
                decInstr.opBranch.reset();
                decInstr.opLdPc.reset();

                auto funct3 = instr(12, 15);

                decInstr.opAlu.isUopUse            <<= 1;
                decInstr.opAlu.isAdd               <<= (funct3 == 0);
                decInstr.opAlu.isSub               <<= 0;
                decInstr.opAlu.isXor               <<= (funct3 == 0b100);
                decInstr.opAlu.isOr                <<= (funct3 == 0b110);
                decInstr.opAlu.isAnd               <<= (funct3 == 0b111);
                decInstr.opAlu.isCmpLessThanSign   <<= (funct3 == 0b010);
                decInstr.opAlu.isCmpLessThanUSign  <<= (funct3 == 0b011);
                decInstr.opAlu.isShiftLeftLogical  <<= (funct3 == 0b001);
                decInstr.opAlu.isShiftRightLogical <<= (funct3 == 0b101) & (instr(30) == 0);
                decInstr.opAlu.isShiftRightArith   <<= (funct3 == 0b101) & (instr(30) == 1);


            }
            void doAulPcDecode(Reg& instr, bool needPc = false){
                decInstr.regData[RS_1].reset();
                decInstr.regData[RS_2].reset();
                decInstr.regData[RS_3].reset();
                decInstr.regData[RS_des].setFromRegFile(instr(7, 12));

                decInstr.opLs.reset();
                decInstr.opAlu.reset();
                decInstr.opBranch.reset();
                decInstr.opLdPc.isUopUse <<= 1;
                decInstr.opLdPc.needPc   <<= needPc;

            }

            /** 01*/

            void doOpDecode(Reg& instr){
                decInstr.regData[RS_1].setFromRegFile(instr(15, 20));
                decInstr.regData[RS_2].setFromRegFile(instr(20, 25));
                decInstr.regData[RS_3].reset();
                decInstr.regData[RS_des].setFromRegFile(instr(7, 12));

                decInstr.opLs.reset();
                decInstr.opBranch.reset();
                decInstr.opLdPc.reset();

                auto funct3 = instr(12, 15);

                decInstr.opAlu.isUopUse            <<= 1;
                decInstr.opAlu.isAdd               <<= (funct3 == 0) & (instr(30) == 0);
                decInstr.opAlu.isSub               <<= (funct3 == 0) & (instr(30) == 1);
                decInstr.opAlu.isXor               <<= (funct3 == 0b100);
                decInstr.opAlu.isOr                <<= (funct3 == 0b110);
                decInstr.opAlu.isAnd               <<= (funct3 == 0b111);
                decInstr.opAlu.isCmpLessThanSign   <<= (funct3 == 0b010);
                decInstr.opAlu.isCmpLessThanUSign  <<= (funct3 == 0b011);
                decInstr.opAlu.isShiftLeftLogical  <<= (funct3 == 0b001);
                decInstr.opAlu.isShiftRightLogical <<= (funct3 == 0b101) & (instr(30) == 0);
                decInstr.opAlu.isShiftRightArith   <<= (funct3 == 0b101) & (instr(30) == 1);
            }

            void doLuiDecode(Reg& instr){
                doAulPcDecode(instr);
            }

            /** 11 */



            void doBranchDecode(Reg& instr){
                decInstr.regData[RS_1].setFromRegFile(instr(15, 20));
                decInstr.regData[RS_2].setFromRegFile(instr(20, 25));
                decInstr.regData[RS_3].setFromImm();
                decInstr.regData[RS_des].setFromRegFile(instr(7, 12));

                auto funct3 = instr(12, 15);

                decInstr.opLs.reset();
                decInstr.opAlu.reset();
                decInstr.opBranch.isUopUse <<= 1;
                decInstr.opBranch.extendMode <<= instr(13);
                decInstr.opBranch.isJalR     <<= 0b0;
                decInstr.opBranch.isJal      <<= 0b0;
                decInstr.opBranch.isEq       <<= (funct3 == 0b0);
                decInstr.opBranch.isNEq      <<= (funct3 == 0b001);
                decInstr.opBranch.isLt       <<= (funct3 == 0b100) | (funct3 == 0b110);
                decInstr.opBranch.isGe       <<= (funct3 == 0b101) | (funct3 == 0b111);


                decInstr.opLdPc.reset();
            }

            void doJalRDecode(Reg& instr){

                doJalDecode(instr, true);

            }

            void doJalDecode(Reg& instr, bool reqR1 = false){

                if (reqR1){
                    decInstr.regData[RS_1].setFromRegFile(instr(15, 20));
                }else{
                    decInstr.regData[RS_1].setZero();
                }

                decInstr.regData[RS_2].reset();
                decInstr.regData[RS_3].reset();
                decInstr.regData[RS_des].setFromRegFile(instr(7, 12));

                decInstr.opLs.reset();
                decInstr.opAlu.reset();

                decInstr.opBranch.isUopUse   <<= 1;
                decInstr.opBranch.extendMode <<= 0;
                decInstr.opBranch.isJalR     <<= reqR1;
                decInstr.opBranch.isJal      <<= !reqR1;
                decInstr.opBranch.isEq       <<= 0;
                decInstr.opBranch.isNEq      <<= 0;
                decInstr.opBranch.isLt       <<= 0;
                decInstr.opBranch.isGe       <<= 0;


                decInstr.opLdPc.reset();
                decInstr.opLdPc.reset();

            }

//            void doSystemDecode(Reg& instr){
//
//            }

        };


    }

}

#endif //KATHRYN_DECODE_H

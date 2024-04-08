//
// Created by tanawin on 7/4/2567.
//

#ifndef KATHRYN_EXECUTE_H
#define KATHRYN_EXECUTE_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "regAccess.h"

namespace kathryn{

    namespace riscv{

        class Execute{

        public:

            UOp exUop;
            StorageMgmt& _memArb;
            makeWire(reExecute, 1);
            makeReg(dummyReg, XLEN);

            explicit Execute(StorageMgmt& memArb): _memArb(memArb){}

            void flow(RegAccess& regAccess){
                UOp& regAccUop = regAccess.regAccessUop;

                pipBlk{

                    exUop = regAccUop;
                    par {
                        /////////// do load store
                        execLS();
                        /////////// do alu uop
                        execAlu();
                        /////////// do jump uop
                        execJump();
                        //////////// do load pc uop
                        execLdPc();
                    }

                }

            }

            void execLS(){
                cif(exUop.opLs.isUopUse & exUop.opLs.isMemLoad){
                    _memArb.reqStorage(exUop.regData[RS_des].val,
                                       exUop.regData[RS_des].idx, //// todo deal with write index
                                       exUop.regData[RS_des].valid);
                }

                cif (exUop.opLs.isUopUse & (~exUop.opLs.isMemLoad)){
                    _memArb.reqWriteReg(exUop.regData[RS_1].val, exUop.regData[RS_2].val);//// todo deal with write index
                }
            }

            void execAlu(){
                RegEle& rdes = exUop.regData[RS_des];
                RegEle& rs1   = exUop.regData[RS_1];
                RegEle& rs2   = exUop.regData[RS_2];

                /////////////////// do simple alu
                zif(exUop.opAlu.isUopUse){
                    rdes.valid = 1;
                    zif(exUop.opAlu.isAdd){
                        rdes.val = rs1.val + rs2.val;
                    }
                    zif(exUop.opAlu.isSub){
                        rdes.val = rs1.val - rs2.val;
                    }
                    zif(exUop.opAlu.isXor){
                        rdes.val = rs1.val ^ rs2.val;
                    }
                    zif(exUop.opAlu.isOr){
                        rdes.val = rs1.val | rs2.val;
                    }
                    zif(exUop.opAlu.isAnd){
                        rdes.val = rs1.val & rs2.val;
                    }
                    zif(exUop.opAlu.isCmpLessThanSign){
                        rdes.val(1, XLEN) = 0;
                        rdes.val(0) = (rs1.val(XLEN-1) & (~rs2.val(XLEN-1)))   |
                                          (
                                          (rs1.val(XLEN-1) == (rs2.val(XLEN-1))) &
                                          (rs1.val(0, XLEN-1) < rs2.val(0, XLEN-1))
                                          );
                    }
                    zif(exUop.opAlu.isCmpLessThanUSign){
                        rdes.val(1, XLEN) = 0;
                        rdes.val(0) = rs1.val(0, XLEN-1) < rs2.val(0, XLEN-1);
                    }
                }
                /////////////////// do complex task alu

                cif(exUop.opAlu.isShiftLeftLogical){
                    cwhile(rs2.val(0,5) > 0){
                        rdes.val = rdes.val << 1;
                        rs2.val  = rs2.val - 1;
                    }
                }

                cif(exUop.opAlu.isShiftRightLogical){
                    cwhile(rs2.val(0,5) > 0){
                        rdes.val = rdes.val >> 1;
                        rs2.val  = rs2.val - 1;
                    }
                }

                cif(exUop.opAlu.isShiftRightArith){
                    cwhile(rs2.val(0,5) > 0){
                        rdes.val(0, XLEN-1) = rdes.val(0, XLEN-1) >> 1;
                        rs2.val  = rs2.val - 1;
                    }
                }


            }

            void execJump(){

                /**check pc*/

            }

            void execLdPc(){

                /** check pc*/

            }


        };

    }
}

#endif //KATHRYN_EXECUTE_H

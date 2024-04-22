//
// Created by tanawin on 7/4/2567.
//

#ifndef KATHRYN_EXECUTE_H
#define KATHRYN_EXECUTE_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "example/riscv/subSystem/storageMgm.h"
#include "example/numberic/extend.h"

namespace kathryn{

    namespace riscv{

        class Execute{
        public:
            UOp exUop;
            UOp& _decodedUop;
            /*** mem access*/
            StorageMgmt& _memArb;
            makeWire(readEn, 1);
            makeWire(readAddr, XLEN);
            Operable& readFn;
            makeReg(dummyReg, XLEN);
            /*** cmp val*/
            makeWire(cmpLtSign, 1);
            makeWire(cmpLtUnSign, 1);

            explicit Execute(UOp& decodedUop, StorageMgmt& memArb):
            _decodedUop(decodedUop),
            _memArb(memArb),
            readFn(_memArb.addReader(readEn, readAddr(2, XLEN)))
            {
                readAddr    = exUop.regData[RS_1].idx + exUop.regData[RS_3].idx;
                cmpLtSign   = (exUop.regData[RS_1].val(XLEN-1) & (~exUop.regData[RS_2].val(XLEN-1)))   |
                             (
                                     (exUop.regData[RS_1].val(XLEN-1)          == exUop.regData[RS_2].val(XLEN-1)) &
                                     (exUop.regData[RS_1].val(0, XLEN-1) <  exUop.regData[RS_2].val(0, XLEN-1))
                             );
                cmpLtUnSign =  exUop.regData[RS_1].val(0, XLEN-1) < exUop.regData[RS_2].val(0, XLEN-1);
            }

            void accessRegData(int idx, MemBlock& memBlock,BYPASS_DATA& bypassData){
                RegEle& rs1    = _decodedUop.regData[idx];
                RegEle& rsfill = exUop.regData[idx];
                zif( (~rs1.valid) & (rs1.idx != 0)){
                    zif(bypassData.idx == rs1.idx){ rsfill.val <<= bypassData.value;}     /////// bypass?
                    zelse                         { rsfill.val <<= memBlock[rs1.idx];} ///////// fill data
                }
            }



            void flow(Wire& misPredic, Wire& reStartPc, MemBlock& memBlock, BYPASS_DATA& bypassData){

                pipBlk{
                    par{
                        accessRegData(RS_1, memBlock, bypassData); ////// access register 1
                        accessRegData(RS_2,  memBlock, bypassData);
                    }
                    par{

                        /////////// do alu uop
                        execAlu(misPredic, reStartPc);
                    }
                    par{
                        execComplexAlu();
                        /////////// do load store
                        execLS();
                    }
                }
            }

            void execAlu(Wire& misPredic,Wire& reStartPc){
                RegEle& rdes  = exUop.regData[RS_des];
                RegEle& rs1   = exUop.regData[RS_1];
                RegEle& rs2   = exUop.regData[RS_2];
                RegEle& rs3   = exUop.regData[RS_3];
                /////////////////// do simple alu
                zif(exUop.opAlu.isUopUse){
                    rdes.valid <<= 1;
                    zif(exUop.opAlu.isAdd){ rdes.val <<= rs1.val + rs2.val;}
                    zif(exUop.opAlu.isSub){ rdes.val <<= rs1.val - rs2.val;}
                    zif(exUop.opAlu.isXor){ rdes.val <<= rs1.val ^ rs2.val;}
                    zif(exUop.opAlu.isOr) { rdes.val <<= rs1.val | rs2.val;}
                    zif(exUop.opAlu.isAnd){ rdes.val <<= rs1.val & rs2.val;}
                    zif(exUop.opAlu.isCmpLessThanSign) {rdes.val(1, XLEN) <<= 0; rdes.val(0) <<= cmpLtSign;}
                    zif(exUop.opAlu.isCmpLessThanUSign){rdes.val(1, XLEN) <<= 0; rdes.val(0) <<= cmpLtUnSign;}
                }

                zif(exUop.opCtrlFlow.isUopUse){
                    /** this work only if predic pc is eq to pc+4*/

                    misPredic = (exUop.opCtrlFlow.isJal) |
                                (exUop.opCtrlFlow.isJalR)|
                                (exUop.opCtrlFlow.isEq   & rs1.val == rs2.val) |
                                (exUop.opCtrlFlow.isNEq  & rs1.val != rs2.val) |
                                (exUop.opCtrlFlow.isLt   &   exUop.opCtrlFlow.extendMode  & cmpLtSign) |
                                (exUop.opCtrlFlow.isLt   & (~exUop.opCtrlFlow.extendMode)  & cmpLtUnSign) |      //////// unsign
                                (exUop.opCtrlFlow.isGe   & ( exUop.opCtrlFlow.extendMode)  & (~cmpLtSign)) |
                                (exUop.opCtrlFlow.isGe   & (~exUop.opCtrlFlow.extendMode)  & (~cmpLtUnSign));
                    zif(exUop.opCtrlFlow.isJal){
                        reStartPc    = exUop.pc + rs1.val;
                        rdes.val   <<= exUop.pc + 4;
                        rdes.valid <<= 1;
                    }zelif(exUop.opCtrlFlow.isJalR){
                        reStartPc    = rs1.val + rs2.val;
                        rdes.val   <<= exUop.pc + 4;
                        rdes.valid <<= 1;
                    }zelse{
                        reStartPc = exUop.pc + rs3.val;
                    }
                }

                zif(exUop.opLdPc.isUopUse){
                    rdes.valid <<= 1;
                    zif(exUop.opLdPc.needPc){ rdes.val <<= exUop.pc + rs1.val;}
                    zelse{ rdes.val <<= rs1.val; };
                }

            }

            void execComplexAlu(){

                RegEle& rdes  = exUop.regData[RS_des];
                RegEle& rs1   = exUop.regData[RS_1];
                RegEle& rs2   = exUop.regData[RS_2];

                cif(exUop.opAlu.isUopUse) {

                    cif(exUop.opAlu.isShiftLeftLogical) {
                        cdowhile(rs2.val(0, 5) > 1){
                            rdes.val <<= rdes.val << (rs2.val(0, 5) > 0);
                            rs2 .val <<= rs2.val - 1;
                        }
                    }

                    cif(exUop.opAlu.isShiftRightLogical) {
                        cdowhile(rs2.val(0, 5) > 1) {
                            rdes.val <<= rdes.val >> (rs2.val(0, 5) > 0);
                            rs2 .val <<= rs2.val - 1;
                            sbreakCon();
                        }
                    }

                    cif(exUop.opAlu.isShiftRightArith) {
                        cdowhile(rs2.val(0, 5) > 1) {
                            rdes.val(0, XLEN - 1) <<= rdes.val(0, XLEN - 1) >> (rs2.val(0, 5) > 0);
                            rs2 .val <<= rs2.val - 1;
                        }
                    }
                }
            }

            void execLS(){

                cif(exUop.opLs.isUopUse){
                    cdowhile(!readFn){
                        readEn = 1;
                        zif((exUop.opLs.size == 0b00) &readFn) {
                            exUop.regData[RS_des].val <<= getExtendExpr(_memArb.readOutput(0, 8),XLEN,false);
                            zif(~exUop.opLs.isMemLoad) {
                                _memArb.reqWriteReq(g(_memArb.readOutput(8, XLEN), exUop.regData[RS_2].val(0, 8)),readAddr
                                );
                            }
                        }zif((exUop.opLs.size == 0b01) & (readFn)) {
                            exUop.regData[RS_des].val <<= getExtendExpr(_memArb.readOutput(0, 16),XLEN,false);
                            zif(~exUop.opLs.isMemLoad) {
                                _memArb.reqWriteReq(g(_memArb.readOutput(16, XLEN), exUop.regData[RS_2].val(0, 16)),readAddr);
                            }
                        }zif((exUop.opLs.size == 0b10) & (readFn)) {
                            exUop.regData[RS_des].val <<= _memArb.readOutput;
                            zif(~exUop.opLs.isMemLoad) {
                                _memArb.reqWriteReq(exUop.regData[RS_2].val(0, 16),readAddr);
                            }
                        }
                    }
                }
            }

        };

    }
}

#endif //KATHRYN_EXECUTE_H

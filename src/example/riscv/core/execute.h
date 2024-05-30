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
            UOp& _decodedUop;
            RegEle& rdes;
            RegEle& rs1;
            RegEle& rs2;
            RegEle& rs3;
            /*** mem access*/
            StorageMgmt& _memArb;
            mWire(readEn, 1);
            mWire(readAddr, MEM_ADDR_IDX_ACTUAL_AL32);
            Operable& readFn;
            mReg(dummyReg, XLEN);
            /*** cmp val*/
            mWire(cmpLtSign, 1);
            mWire(cmpLtUnSign, 1);

            mWire(testExit, 1);

            mWire(m16, 1);

            FlowBlockBase* regAccessBlock = nullptr;
            FlowBlockBase* aluBlock       = nullptr;
            FlowBlockBase* complexExe     = nullptr;

            explicit Execute(UOp& decodedUop, StorageMgmt& memArb, RegEle& desReg):
            _decodedUop(decodedUop),
            rdes(desReg),
            rs1(_decodedUop.regData[RS_1]),
            rs2(_decodedUop.regData[RS_2]),
            rs3(_decodedUop.regData[RS_3]),
            _memArb(memArb),
            readFn(_memArb.addReader(readEn, readAddr))
            {
                readAddr    = (_decodedUop.regData[RS_1].val + _decodedUop.regData[RS_3].val)(MEM_ADDR_SL);
                cmpLtSign   = (_decodedUop.regData[RS_1].val(XLEN - 1) & (~_decodedUop.regData[RS_2].val(XLEN - 1))) |
                              (
                                      (_decodedUop.regData[RS_1].val(XLEN - 1) == _decodedUop.regData[RS_2].val(XLEN - 1)) &
                                      (_decodedUop.regData[RS_1].val(0, XLEN - 1) < _decodedUop.regData[RS_2].val(0, XLEN - 1))
                             );
                cmpLtUnSign = _decodedUop.regData[RS_1].val < _decodedUop.regData[RS_2].val;
            }

            void accessRegData(int idx, MemBlock& memBlock,BYPASS_DATA& bypassData){
                RegEle& rsx    = _decodedUop.regData[idx];
                zif(~rsx.valid){
                    rsx.valid <<= 1;
                    zif(rsx.idx != 0) {
                        zif(bypassData.idx == rsx.idx) { rsx.val <<= bypassData.value; }     /////// bypass?
                        zelse { rsx.val <<= memBlock[rsx.idx]; } ///////// fill data
                    }zelse{
                        rsx.val   <<= 0;
                    }
                }
            }



            void flow(Wire& misPredic, Wire& reStartPc, MemBlock& memBlock, BYPASS_DATA& bypassData){

                pipBlk{
                    par{ exposeBlk(regAccessBlock)
                        accessRegData(RS_1, memBlock, bypassData); ////// access register 1
                        accessRegData(RS_2,  memBlock, bypassData);
                        rdes <<= _decodedUop.regData[RS_des];
                    }
                    par{ exposeBlk(aluBlock)
                        execAlu(misPredic, reStartPc);
                    }
                    par{ exposeBlk(complexExe)
                        execComplexAlu(); execLS();
                    }
                }
            }

            void execAlu(Wire& misPredic,Wire& reStartPc){
                /////////////////// do simple alu
                zif(_decodedUop.opAlu.isUopUse){
                    rdes.valid <<= 1;
                    zif(_decodedUop.opAlu.isAdd){ rdes.val <<= rs1.val + rs2.val;}
                    zif(_decodedUop.opAlu.isSub){ rdes.val <<= rs1.val - rs2.val;}
                    zif(_decodedUop.opAlu.isXor){ rdes.val <<= rs1.val ^ rs2.val;}
                    zif(_decodedUop.opAlu.isOr) { rdes.val <<= rs1.val | rs2.val;}
                    zif(_decodedUop.opAlu.isAnd){ rdes.val <<= rs1.val & rs2.val;}
                    zif(_decodedUop.opAlu.isCmpLessThanSign)  { rdes.val(1, XLEN) <<= 0; rdes.val(0) <<= cmpLtSign;}
                    zif(_decodedUop.opAlu.isCmpLessThanUSign) { rdes.val(1, XLEN) <<= 0; rdes.val(0) <<= cmpLtUnSign;}
                    zif(_decodedUop.opAlu.isShiftLeftLogical |
                        _decodedUop.opAlu.isShiftRightArith  |
                        _decodedUop.opAlu.isShiftRightLogical) { rdes.val <<= rs1.val;}
                }

                zif(_decodedUop.opCtrlFlow.isUopUse){
                    /** this work only if predic pc is eq to pc+4*/

                    misPredic = (_decodedUop.opCtrlFlow.isJal) |
                                (_decodedUop.opCtrlFlow.isJalR) |
                                (_decodedUop.opCtrlFlow.isEq & rs1.val == rs2.val) |
                                (_decodedUop.opCtrlFlow.isNEq & rs1.val != rs2.val) |
                                (_decodedUop.opCtrlFlow.isLt & (_decodedUop.opCtrlFlow.extendMode ) & cmpLtUnSign)  |
                                (_decodedUop.opCtrlFlow.isLt & (~_decodedUop.opCtrlFlow.extendMode) & cmpLtSign)    |      //////// unsign
                                (_decodedUop.opCtrlFlow.isGe & ( _decodedUop.opCtrlFlow.extendMode) & (~cmpLtUnSign)) |
                                (_decodedUop.opCtrlFlow.isGe & (~_decodedUop.opCtrlFlow.extendMode) & (~cmpLtSign));
                    zif(_decodedUop.opCtrlFlow.isJal){
                        reStartPc    = _decodedUop.pc + rs2.val;
                        rdes.val   <<= _decodedUop.pc + 4;
                        rdes.valid <<= 1;
                    }zelif(_decodedUop.opCtrlFlow.isJalR){
                        reStartPc    = rs1.val + rs2.val;
                        rdes.val   <<= _decodedUop.pc + 4;
                        rdes.valid <<= 1;
                    }zelse{
                        reStartPc = _decodedUop.pc + rs3.val;
                    }
                }

                zif(_decodedUop.opLdPc.isUopUse){
                    rdes.valid <<= 1;
                    zif(_decodedUop.opLdPc.needPc){ rdes.val <<= (_decodedUop.pc + rs2.val);}
                    zelse                         { rdes.val <<= rs2.val; };
                }

            }

            void execComplexAlu(){

                cif(_decodedUop.opAlu.isUopUse) {

                    cif(_decodedUop.opAlu.isShiftLeftLogical) {
                        cdowhile(rs2.val(0, 5) > 1){
                            rdes.val <<= rdes.val << (rs2.val(0, 5) > 0);
                            zif(rs2.val(0, 5) > 1) {
                                rs2.val <<= rs2.val - 1;
                            }
                        }
                    }

                    cif(_decodedUop.opAlu.isShiftRightLogical) {
                        cdowhile(rs2.val(0, 5) > 1) {
                            rdes.val <<= rdes.val >> (rs2.val(0, 5) > 0);
                            zif(rs2.val(0, 5) > 1) {
                                rs2.val <<= rs2.val - 1;
                            }
                        }
                    }

                    cif(_decodedUop.opAlu.isShiftRightArith) {
                        cdowhile(rs2.val(0, 5) > 1) {
                            rdes.val(0, XLEN - 1) <<= rdes.val(0, XLEN - 1) >> (rs2.val(0, 5) > 0);
                            zif(rs2.val(0, 5) > 1) {
                                rs2.val <<= rs2.val - 1;
                            }
                        }
                    }
                }
            }

            void execLS(){

                cif(_decodedUop.opLs.isUopUse){
                    cdowhile(!readFn) {
                        readEn = 1;
                        rdes.valid <<= _decodedUop.opLs.isMemLoad;
                        zif((_decodedUop.opLs.size == 0b00) & readFn) {
                            zif(_decodedUop.opLs.extendMode) { ////sign extend
                                rdes.val <<= getExtendExpr(_memArb.readOutput(0, 8), XLEN, true);
                            }zelse{
                                rdes.val <<= getExtendExpr(_memArb.readOutput(0, 8), XLEN, false);
                            }
                            zif(~_decodedUop.opLs.isMemLoad) {
                                _memArb.reqWriteReq(
                                        g(_memArb.readOutput(8, XLEN), _decodedUop.regData[RS_2].val(0, 8)),
                                        readAddr
                                );
                            }
                        }
                        zif((_decodedUop.opLs.size == 0b01) & (readFn)) {
                            zif(_decodedUop.opLs.extendMode) { ////sign extend
                                rdes.val <<= getExtendExpr(_memArb.readOutput(0, 16), XLEN, true);
                            }zelse{
                                rdes.val <<= getExtendExpr(_memArb.readOutput(0, 16), XLEN, false);
                            }
                            zif(~_decodedUop.opLs.isMemLoad) {
                                _memArb.reqWriteReq(
                                        g(_memArb.readOutput(16, XLEN), _decodedUop.regData[RS_2].val(0, 16)),
                                        readAddr);
                            }
                        }
                        zif((_decodedUop.opLs.size == 0b10) & (readFn)) {
                            rdes.val <<= _memArb.readOutput;
                            zif(~_decodedUop.opLs.isMemLoad) {
                                _memArb.reqWriteReq(_decodedUop.regData[RS_2].val, readAddr);
                            }
                        }
                    }
                }
            }

        };

    }
}

#endif //KATHRYN_EXECUTE_H

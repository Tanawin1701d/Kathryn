//
// Created by tanawin on 7/4/2567.
//

#ifndef KATHRYN_EXECUTE_H
#define KATHRYN_EXECUTE_H

#include "kathryn.h"
#include "lib/instr/instrBase.h"
#include "example/riscv/element.h"
#include "example/riscv/subSystem/storageMgm.h"
#include "example/numberic/extend.h"

namespace kathryn{

    namespace riscv{

        class Execute{
        public:
            UOp&    _decodedUop;
            OPR_HW& rdes;
            OPR_HW& rs0;
            OPR_HW& rs1;
            OPR_HW& rs2;
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

            explicit Execute(UOp& decodedUop, StorageMgmt& memArb, OPR_HW& desReg):
            _decodedUop(decodedUop),
            rdes(desReg),
            rs0(_decodedUop.repo.getSrcReg(0)),
            rs1(_decodedUop.repo.getSrcReg(1)),
            rs2(_decodedUop.repo.getSrcReg(2)),
            _memArb(memArb),
            readFn(_memArb.addReader(readEn, readAddr))
            {
                readAddr    = (rs0.data + rs2.data)(MEM_ADDR_SL);
                cmpLtSign   = (rs0.data(XLEN - 1) & (~rs1.data(XLEN - 1))) |
                              (
                                      (rs0.data(XLEN - 1) == rs1.data(XLEN - 1)) &
                                      (rs0.data(0, XLEN - 1) < rs1.data(0, XLEN - 1))
                             );
                cmpLtUnSign = rs0.data < rs1.data;
            }

            void accessRegData(OPR_HW& rsx, MemBlock& memBlock,BYPASS_DATA& bypassData){
                zif(~rsx.valid){
                    rsx.valid <<= 1;
                    zif(rsx.idx != 0) {
                        zif(bypassData.idx == rsx.idx) { rsx.data <<= bypassData.value; }     /////// bypass?
                        zelse { rsx.data <<= memBlock[rsx.idx]; } ///////// fill data
                    }zelse{
                        rsx.data   <<= 0;
                    }
                }
            }



            void flow(Wire& misPredic, Wire& reStartPc, MemBlock& memBlock, BYPASS_DATA& bypassData){

                // pipBlk{
                //     par{ exposeBlk(regAccessBlock)
                //         accessRegData(rs0, memBlock, bypassData); ////// access register 1
                //         accessRegData(rs1,  memBlock, bypassData);
                //         rdes <<= _decodedUop.repo.getDesReg(0);
                //     }
                //     par{ exposeBlk(aluBlock)
                //         execAlu(misPredic, reStartPc);
                //     }
                //     par{ exposeBlk(complexExe)
                //         execComplexAlu(); execLS();
                //     }
                // }
            }

            void execAlu(Wire& misPredic,Wire& reStartPc){
                /////////////////// do simple alu
                auto mop = _decodedUop.repo.getOp("op");
                zif(mop.isSet()){
                    rdes.valid <<= 1;
                    zif(mop.isUopSet("add")  /*.isAdd*/                   ){ rdes.data <<= rs0.data + rs1.data;}
                    zif(mop.isUopSet("sub")  /*.isSub*/                   ){ rdes.data <<= rs0.data - rs1.data;}
                    zif(mop.isUopSet("xor")  /*.isXor*/                   ){ rdes.data <<= rs0.data ^ rs1.data;}
                    zif(mop.isUopSet("or")   /*.isOr*/                    ){ rdes.data <<= rs0.data | rs1.data;}
                    zif(mop.isUopSet("and")  /*.isAnd*/                   ){ rdes.data <<= rs0.data & rs1.data;}
                    zif(mop.isUopSet("slt")  /*.isCmpLessThanSign*/       ){ rdes.data(1, XLEN) <<= 0; rdes.data(0) <<= cmpLtSign;}
                    zif(mop.isUopSet("sltu") /*.isCmpLessThanUSign*/      ){ rdes.data(1, XLEN) <<= 0; rdes.data(0) <<= cmpLtUnSign;}
                    zif(mop.isUopSet("sll")  /*.isShiftLeftLogical*/  |
                        mop.isUopSet("sr")  /*.isShiftRightArith*/)        { rdes.data <<= rs1.data;}
                }

                auto bmop = _decodedUop.repo.getOp("opImm");
                zif(bmop.isSet()){
                    /** this work only if predic pc is eq to pc+4*/

                    misPredic = (bmop.isUopSet("jal")/* _decodedUop.opCtrlFlow.isJal*/     ) |
                                (bmop.isUopSet("jalr")/* _decodedUop.opCtrlFlow.isJalR*/     ) |
                                (bmop.isUopSet("beq")/* _decodedUop.opCtrlFlow.isEq*/       & rs0.data == rs1.data) |
                                (bmop.isUopSet("bne")/* _decodedUop.opCtrlFlow.isNEq*/      & rs0.data != rs1.data) |
                                (bmop.isUopSet("bltx")/* _decodedUop.opCtrlFlow.isLt*/      & ( bmop.isUopSet("signMode")) & cmpLtUnSign)  |      //////// sign mode
                                (bmop.isUopSet("bltx")/* _decodedUop.opCtrlFlow.isLt*/      & (~bmop.isUopSet("signMode")) & cmpLtSign)    |      //////// unsign
                                (bmop.isUopSet("bgex")/* _decodedUop.opCtrlFlow.isGe*/      & ( bmop.isUopSet("signMode")) & (~cmpLtUnSign)) |
                                (bmop.isUopSet("bgex")/* _decodedUop.opCtrlFlow.isGe*/      & (~bmop.isUopSet("signMode")) & (~cmpLtSign));
                    zif(bmop.isUopSet("jal")){
                        reStartPc    = _decodedUop.pc + rs1.data;
                        rdes.data   <<= _decodedUop.pc + 4;
                        rdes.valid  <<= 1;
                    }zelif(bmop.isUopSet("jalr")){
                        reStartPc    = rs0.data + rs1.data;
                        rdes.data  <<= _decodedUop.pc + 4;
                        rdes.valid <<= 1;
                    }zelse{
                        reStartPc = _decodedUop.pc + rs2.data;
                    }
                }
                auto ldMop = _decodedUop.repo.getOp("ldPc");
                zif(ldMop.isSet()){
                    rdes.valid <<= 1;
                    zif(ldMop.isUopSet("needPc")){rdes.data <<= (_decodedUop.pc + rs1.data);}
                    zelse                        {rdes.data <<= rs1.data;}
                }

            }

            void execComplexAlu(){

                // cif(_decodedUop.opAlu.isUopUse) {
                //
                //     cif(_decodedUop.opAlu.isShiftLeftLogical) {
                //         cdowhile(rs2.val(0, 5) > 1){
                //             rdes.val <<= rdes.val << (rs2.val(0, 5) > 0);
                //             zif(rs2.val(0, 5) > 1) {
                //                 rs2.val <<= rs2.val - 1;
                //             }
                //         }
                //     }
                //
                //     cif(_decodedUop.opAlu.isShiftRightLogical) {
                //         cdowhile(rs2.val(0, 5) > 1) {
                //             rdes.val <<= rdes.val >> (rs2.val(0, 5) > 0);
                //             zif(rs2.val(0, 5) > 1) {
                //                 rs2.val <<= rs2.val - 1;
                //             }
                //         }
                //     }
                //
                //     cif(_decodedUop.opAlu.isShiftRightArith) {
                //         cdowhile(rs2.val(0, 5) > 1) {
                //             rdes.val(0, XLEN - 1) <<= rdes.val(0, XLEN - 1) >> (rs2.val(0, 5) > 0);
                //             zif(rs2.val(0, 5) > 1) {
                //                 rs2.val <<= rs2.val - 1;
                //             }
                //         }
                //     }
                // }
            }

            void execLS(){

                // cif(_decodedUop.opLs.isUopUse){
                //     cdowhile(!readFn) {
                //         readEn = 1;
                //         rdes.valid <<= _decodedUop.opLs.isMemLoad;
                //         zif((_decodedUop.opLs.size == 0b00) & readFn) {
                //             zif(_decodedUop.opLs.extendMode) { ////sign extend
                //                 rdes.val <<= getExtendExpr(_memArb.readOutput(0, 8), XLEN, true);
                //             }zelse{
                //                 rdes.val <<= getExtendExpr(_memArb.readOutput(0, 8), XLEN, false);
                //             }
                //             zif(~_decodedUop.opLs.isMemLoad) {
                //                 _memArb.reqWriteReq(
                //                         g(_memArb.readOutput(8, XLEN), _decodedUop.regData[RS_2].val(0, 8)),
                //                         readAddr
                //                 );
                //             }
                //         }
                //         zif((_decodedUop.opLs.size == 0b01) & (readFn)) {
                //             zif(_decodedUop.opLs.extendMode) { ////sign extend
                //                 rdes.val <<= getExtendExpr(_memArb.readOutput(0, 16), XLEN, true);
                //             }zelse{
                //                 rdes.val <<= getExtendExpr(_memArb.readOutput(0, 16), XLEN, false);
                //             }
                //             zif(~_decodedUop.opLs.isMemLoad) {
                //                 _memArb.reqWriteReq(
                //                         g(_memArb.readOutput(16, XLEN), _decodedUop.regData[RS_2].val(0, 16)),
                //                         readAddr);
                //             }
                //         }
                //         zif((_decodedUop.opLs.size == 0b10) & (readFn)) {
                //             rdes.val <<= _memArb.readOutput;
                //             zif(~_decodedUop.opLs.isMemLoad) {
                //                 _memArb.reqWriteReq(_decodedUop.regData[RS_2].val, readAddr);
                //             }
                //         }
                //     }
                // }
            }

        };

    }
}

#endif //KATHRYN_EXECUTE_H

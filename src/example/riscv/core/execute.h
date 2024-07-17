//
// Created by tanawin on 7/4/2567.
//

#ifndef KATHRYN_EXECUTE_H
#define KATHRYN_EXECUTE_H

#include "kathryn.h"
#include "lib/instr/instrBase.h"
#include "example/riscv/element.h"
#include "example/riscv/subSystem/storageMgm.h"
#include "lib/numberic/numberic.h"

namespace kathryn{

    namespace riscv{

        class Execute{
        public:
            UOp&    _decodedUop;
            OPR_HW& rdes;
            OPR_HW& rs1;
            OPR_HW& rs2;
            OPR_HW& rs3;
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

            FlowBlockBase* regAccessBlock = nullptr;
            FlowBlockBase* aluBlock       = nullptr;
            FlowBlockBase* complexExe     = nullptr;

            explicit Execute(UOp& decodedUop, StorageMgmt& memArb, OPR_HW& desReg):
            _decodedUop(decodedUop),
            rdes(desReg),
            rs1(_decodedUop.repo.getSrcReg(0)),
            rs2(_decodedUop.repo.getSrcReg(1)),
            rs3(_decodedUop.repo.getSrcReg(2)),
            _memArb(memArb),
            readFn(_memArb.addReader(readEn, readAddr))
            {
                readAddr    = (rs1.data + rs3.data)(MEM_ADDR_SL);
                cmpLtSign   = (rs1.data(XLEN - 1) & (~rs2.data(XLEN - 1))) |
                              (
                                      (rs1.data(XLEN - 1) == rs2.data(XLEN - 1)) &
                                      (rs1.data(0, XLEN - 1) < rs2.data(0, XLEN - 1))
                             );
                cmpLtUnSign = rs1.data < rs2.data;
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

                pipBlk{
                    par{ exposeBlk(regAccessBlock)
                        accessRegData(rs1, memBlock, bypassData); ////// access register 1
                        accessRegData(rs2,  memBlock, bypassData);
                        rdes <<= _decodedUop.repo.getDesReg(0);
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
                auto mop = _decodedUop.repo.getOp("op");
                zif(mop.isSet()){
                    rdes.valid <<= 1;
                    zif(mop.isUopSet("add" )){ rdes.data <<= rs1.data + rs2.data;}
                    zif(mop.isUopSet("sub" )){ rdes.data <<= rs1.data - rs2.data;}
                    zif(mop.isUopSet("xor" )){ rdes.data <<= rs1.data ^ rs2.data;}
                    zif(mop.isUopSet("or"  )){ rdes.data <<= rs1.data | rs2.data;}
                    zif(mop.isUopSet("and" )){ rdes.data <<= rs1.data & rs2.data;}
                    zif(mop.isUopSet("slt" )){ rdes.data(1, XLEN) <<= 0; rdes.data(0) <<= cmpLtSign;}
                    zif(mop.isUopSet("sltu")){ rdes.data(1, XLEN) <<= 0; rdes.data(0) <<= cmpLtUnSign;}
                    zif(mop.isUopSet("sll") | mop.isUopSet("sr") | mop.isUopSet("sra")){ rdes.data <<= rs1.data;}
                }

                auto bmop = _decodedUop.repo.getOp("br");
                auto jmop = _decodedUop.repo.getOp("jal");
                zif(bmop.isSet() | jmop.isSet()){
                    /** this work only if predic pc is eq to pc+4*/

                    misPredic = (jmop.isSet()) |
                                (bmop.isUopSet("beq") & rs1.data == rs2.data) |
                                (bmop.isUopSet("bne") & rs1.data != rs2.data) |
                                (bmop.isUopSet("blt") & cmpLtSign    )  |      //////// sign mode
                                (bmop.isUopSet("bltu")& cmpLtUnSign  )  |      //////// unsign
                                (bmop.isUopSet("bge") & (~cmpLtSign  )) |
                                (bmop.isUopSet("bgeu")& (~cmpLtUnSign));
                    zif(jmop.isSet()){
                        zif(jmop.isUopSet("needpc")){
                            reStartPc   = _decodedUop.pc + rs2.data;
                            rdes.data   <<= _decodedUop.pc + 4;
                            rdes.valid  <<= 1;
                        }zelse{
                            reStartPc  =  rs1.data + rs2.data;
                            rdes.data  <<= _decodedUop.pc + 4;
                            rdes.valid <<= 1;
                        }
                    }zelse{
                        reStartPc = _decodedUop.pc + rs3.data;
                    }
                }
                auto ldMop = _decodedUop.repo.getOp("ldpc");
                zif(ldMop.isSet()){
                    rdes.valid <<= 1;
                    zif(ldMop.isUopSet("needpc")){rdes.data <<= (_decodedUop.pc + rs2.data);}
                    zelse                        {rdes.data <<= rs2.data;}
                }

            }

            void execComplexAlu(){
                auto op = _decodedUop.repo.getOp("op");
                cif(op.isSet()) {
                    cif(op.isUopSet("sll") | op.isUopSet("sra") |
                        op.isUopSet("sr")){
                        cdowhile(rs2.data(0, 5) > 1){
                            zif (op.isUopSet("sll")){ rdes.data <<= rdes.data << (rs2.data(0, 5) > 0);}
                            zif (op.isUopSet("sra")){
                                rdes.data(0, XLEN - 1) <<= (rdes.data(0, XLEN - 1) >> (rs2.data(0, 5) > 0));
                            }
                            zif (op.isUopSet("sr")){ rdes.data <<= rdes.data >> (rs2.data(0, 5) > 0);}

                            zif(rs2.data(0, 5) > 1) { rs2.data <<= rs2.data - 1;}
                        }
                    }
                }
            }

            void execLS(){
                auto ldst = _decodedUop.repo.getOp("ldst");
                Reg& usign      = ldst.isUopSet("usign");
                mWire(poolWriteData, XLEN);
                mWire(finReadData,   XLEN);

                cif(ldst.isSet()){
                    cdowhile(!readFn) {
                        readEn = 1;
                        rdes.valid <<= ldst.isUopSet("isload");

                        zif(ldst.isUopSet("lsb")){
                            poolWriteData = g(_memArb.readOutput(8,XLEN), rs2.data(0,8));
                            finReadData   = ext(_memArb.readOutput(0, 8), XLEN, ~usign);

                        }
                        zif(ldst.isUopSet("lsh")){
                            poolWriteData = g(_memArb.readOutput(16,XLEN), rs2.data(0,16));
                            finReadData   = ext(_memArb.readOutput(0, 16), XLEN, ~usign);
                        }
                        zif(ldst.isUopSet("lsw")){
                            poolWriteData = rs2.data;
                            finReadData   = _memArb.readOutput;
                        }
                        rdes.data <<= finReadData;
                        _memArb.reqWriteReq(~ldst.isUopSet("isload"),readAddr, poolWriteData);
                    }
                }
            }

        };

    }
}

#endif //KATHRYN_EXECUTE_H

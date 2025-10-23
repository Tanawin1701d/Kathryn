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

namespace kathryn::riscv{

        struct Execute{
            CORE_DATA& cd;
            DECODE_DATA& decData;
            OPR_HW& rdes;
            OPR_HW& rs1;
            OPR_HW& rs2;
            OPR_HW& rs3;
            /*** mem access*/
            StorageMgmt& _memArb;
            mWire(readEn, 1);
            mWire(readAddr, MEM_ADDR_IDX_ACTUAL_AL32);
            mWire(misPredic, 1);
            Operable& readFn;
            mReg(dummyReg, XLEN);
            /*** cmp val*/
            mWire(cmpLtSign, 1);
            mWire(cmpLtUnSign, 1);

            PipSimProbe pipSimProbe;
            SimProbe    acRegSimProb;
            SimProbe    aluSimProb;
            SimProbe    complexAluSimProb;



            explicit Execute(CORE_DATA& coreData, StorageMgmt& memArb):
            cd(coreData),
            decData(cd.dc),
            rdes(cd.ex.wbData),
            rs1(cd.dc.repo.getSrcReg(0)),
            rs2(cd.dc.repo.getSrcReg(1)),
            rs3(cd.dc.repo.getSrcReg(2)),
            _memArb(memArb),
            readFn(_memArb.addReader(readEn, readAddr))
            {
                readAddr    = (rs1.data + rs3.data)(MEM_ADDR_SL);
                // cmpLtSign   = (rs1.data(XLEN - 1) & (~rs2.data(XLEN - 1))) |
                //               (
                //                       (rs1.data(XLEN - 1) == rs2.data(XLEN - 1)) &
                //                       (rs1.data(0, XLEN - 1) < rs2.data(0, XLEN - 1))
                //              );
                cmpLtSign   = rs1.data.slt(rs2.data);
                cmpLtUnSign = rs1.data < rs2.data;
            }

            void accessRegData(OPR_HW& rsx, MemBlock& memBlock){

                zif(~rsx.valid){
                    rsx.valid <<= 1;
                    rsx.data  <<= 0;
                    ///// it is supposed to be writeback in this cycle
                    zif(rsx.idx != 0) {
                        zif(cd.bp.idx == rsx.idx) { rsx.data <<= cd.bp.value; }     /////// bypass?
                        zelse { rsx.data <<= memBlock[rsx.idx]; } ///////// fill data
                    }
                }
            }



            void flow(MemBlock& memBlock){

                pip(cd.ex.sync){ initProbe(pipSimProbe);
                    seq{
                        par{ initProbe(acRegSimProb);
                            accessRegData(rs1, memBlock); ////// access register 1
                            accessRegData(rs2,  memBlock);
                            rdes <<= decData.repo.getDesReg(0);
                        }
                        par{ initProbe(aluSimProb);
                            execAlu();
                        }
                        par{ initProbe(complexAluSimProb);
                            pick{
                                execComplexAlu(); execLS();
                                pickDef
                            }
                        }
                    }
                }

                //////// sync manually without
                cd.wb.sync.setMasterReady(cd.ex.sync.isSlaveFin());


            }

            void execAlu(){
                /////////////////// do simple alu
                auto mop = decData.repo.getOp("op");
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

                auto bmop = decData.repo.getOp("br");
                auto jmop = decData.repo.getOp("jal");
                zif(bmop.isSet() | jmop.isSet()){
                    /** this work only if predic pc is eq to pc+4*/
                    misPredic = (jmop.isSet()) |
                                (bmop.isUopSet("beq") & rs1.data == rs2.data) |
                                (bmop.isUopSet("bne") & rs1.data != rs2.data) |
                                (bmop.isUopSet("blt") & cmpLtSign    )  |      //////// sign mode
                                (bmop.isUopSet("bltu")& cmpLtUnSign  )  |      //////// unsign
                                (bmop.isUopSet("bge") & (~cmpLtSign  )) |
                                (bmop.isUopSet("bgeu")& (~cmpLtUnSign));
                    //////// kill the system
                    zif(misPredic){cd.kill();}
                    zif(jmop.isSet()){
                        zif(jmop.isUopSet("needpc")){
                            //jdebugNeedPc = 1;
                            cd.changePc(decData.pc + rs2.data);
                            rdes.data   <<= decData.pc + 4;
                            rdes.valid  <<= 1;
                        }zelse{
                            //jdebugNotNeedPc = 1;
                            cd.changePc(rs1.data + rs2.data);
                            rdes.data  <<= decData.pc + 4;
                            rdes.valid <<= 1;
                        }
                    }
                    zif(bmop.isSet() && misPredic){
                        //jdebugNormPc = 1;
                        cd.changePc(decData.pc + rs3.data);
                    }
                }
                auto ldMop = decData.repo.getOp("ldpc");
                zif(ldMop.isSet()){
                    rdes.valid <<= 1;
                    zif(ldMop.isUopSet("needpc")){rdes.data <<= (decData.pc + rs2.data);}
                    zelse                        {rdes.data <<= rs2.data;}
                }

            }

            void execComplexAlu(){
                auto op = decData.repo.getOp("op");
                pif(op.isSet() & (op.isUopSet("sll") | op.isUopSet("sra") |
                        op.isUopSet("sr"))) {
                    cdowhile(rs2.data(0, 5) > 1){
                        zif (op.isUopSet("sll")){ rdes.data <<= rdes.data << (rs2.data(0, 5) > 0);}
                        zif (op.isUopSet("sra")){
                            rdes.data(0, XLEN - 1) <<= (rdes.data(0, XLEN) >> (rs2.data(0, 5) > 0));
                        }
                        zif (op.isUopSet("sr")){ rdes.data <<= rdes.data >> (rs2.data(0, 5) > 0);}

                        zif(rs2.data(0, 5) > 1) { rs2.data <<= rs2.data - 1;}
                    }

                }
            }

            void execLS(){
                auto ldst = decData.repo.getOp("ldst");
                Reg& usign      = ldst.isUopSet("usign");
                mWire(poolWriteData, XLEN);
                mWire(finReadData,   XLEN);

                pif(ldst.isSet()){
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

#endif //KATHRYN_EXECUTE_H

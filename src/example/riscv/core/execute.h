//
// Created by tanawin on 7/4/2567.
//

#ifndef KATHRYN_EXECUTE_H
#define KATHRYN_EXECUTE_H

#include "kathryn.h"
#include "lib/instr/instr_base.h"
#include "example/riscv/element.h"
#include "example/riscv/sub_system/storage_mgm.h"
#include "lib/numberic/numberic.h"

namespace kathryn::riscv{

        struct Execute{
            CORE_DATA& cd;
            DECODE_DATA& dec_data;
            OPR_HW& rdes;
            OPR_HW& rs1;
            OPR_HW& rs2;
            OPR_HW& rs3;
            /*** mem access*/
            StorageMgmt& _memArb;
            m_wire(read_en, 1);
            m_wire(read_addr, MEM_ADDR_IDX_ACTUAL_AL32);
            m_wire(mis_predic, 1);
            Operable& read_fn;
            m_reg(dummy_reg, XLEN);
            /*** cmp val*/
            m_wire(cmp_lt_sign, 1);
            m_wire(cmp_lt_un_sign, 1);

            PipSimProbe pip_sim_probe;
            SimProbe    ac_reg_sim_prob;
            SimProbe    alu_sim_prob;
            SimProbe    complex_alu_sim_prob;



            explicit Execute(CORE_DATA& core_data, StorageMgmt& mem_arb):
            cd(core_data),
            dec_data(cd.dc),
            rdes(cd.ex.wb_data),
            rs1(cd.dc.repo.get_src_reg(0)),
            rs2(cd.dc.repo.get_src_reg(1)),
            rs3(cd.dc.repo.get_src_reg(2)),
            _memArb(mem_arb),
            read_fn(_memArb.add_reader(read_en, read_addr))
            {
                read_addr    = (rs1.data + rs3.data)(MEM_ADDR_SL);
                // cmp_lt_sign   = (rs1.data(XLEN - 1) & (~rs2.data(XLEN - 1))) |
                //               (
                //                       (rs1.data(XLEN - 1) == rs2.data(XLEN - 1)) &
                //                       (rs1.data(0, XLEN - 1) < rs2.data(0, XLEN - 1))
                //              );
                cmp_lt_sign   = rs1.data.slt(rs2.data);
                cmp_lt_un_sign = rs1.data < rs2.data;
            }

            void access_reg_data(OPR_HW& rsx, MemBlock& mem_block){

                zif(~rsx.valid){
                    rsx.valid <<= 1;
                    rsx.data  <<= 0;
                    ///// it is supposed to be writeback in this cycle
                    zif(rsx.idx != 0) {
                        zif(cd.bp.idx == rsx.idx) { rsx.data <<= cd.bp.value; }     /////// bypass?
                        zelse { rsx.data <<= mem_block[rsx.idx]; } ///////// fill data
                    }
                }
            }



            void flow(MemBlock& mem_block){

                pip(cd.ex.sync){ init_probe(pip_sim_probe);
                    seq{
                        par{ init_probe(ac_reg_sim_prob);
                            access_reg_data(rs1, mem_block); ////// access register 1
                            access_reg_data(rs2,  mem_block);
                            rdes <<= dec_data.repo.get_des_reg(0);
                            // m_wire(dbg_st_ex_reg, 1);
                            // dbg_st_ex_reg = 1;
                            // dbg_st_ex_reg.as_output_glob("st_regAccess");
                        }
                        par{ init_probe(alu_sim_prob);
                            exec_alu();
                            // m_wire(dbg_st_ex_alu, 1);
                            // dbg_st_ex_alu = 1;
                            // dbg_st_ex_alu.as_output_glob("st_alu");
                        }
                        par{ init_probe(complex_alu_sim_prob);
                            pick{
                                exec_complex_alu(); exec_ls();
                                pick_def
                            }
                        }
                    }
                }

                //////// sync manually without
                cd.wb.sync.set_master_ready(cd.ex.sync.is_slave_fin());

                // m_wire(dbg_slaveExecFin, 1);
                // dbg_slaveExecFin.as_output_glob("dbg_slaveExecFin");
                // dbg_slaveExecFin = cd.ex.sync.is_slave_fin();


            }

            void exec_alu(){
                /////////////////// do simple alu
                auto mop = dec_data.repo.get_op("op");
                zif(mop.is_set()){
                    rdes.valid <<= 1;
                    zif(mop.is_uop_set("add" )){ rdes.data <<= rs1.data + rs2.data;}
                    zif(mop.is_uop_set("sub" )){ rdes.data <<= rs1.data - rs2.data;}
                    zif(mop.is_uop_set("xor" )){ rdes.data <<= rs1.data ^ rs2.data;}
                    zif(mop.is_uop_set("or"  )){ rdes.data <<= rs1.data | rs2.data;}
                    zif(mop.is_uop_set("and" )){ rdes.data <<= rs1.data & rs2.data;}
                    zif(mop.is_uop_set("slt" )){ rdes.data(1, XLEN) <<= 0; rdes.data(0) <<= cmp_lt_sign;}
                    zif(mop.is_uop_set("sltu")){ rdes.data(1, XLEN) <<= 0; rdes.data(0) <<= cmp_lt_un_sign;}
                    zif(mop.is_uop_set("sll") | mop.is_uop_set("sr") | mop.is_uop_set("sra")){ rdes.data <<= rs1.data;}
                }

                auto bmop = dec_data.repo.get_op("br");
                auto jmop = dec_data.repo.get_op("jal");
                zif(bmop.is_set() | jmop.is_set()){
                    /** this work only if predic pc is eq to pc+4*/
                    mis_predic = (jmop.is_set()) |
                                (bmop.is_uop_set("beq") & rs1.data == rs2.data) |
                                (bmop.is_uop_set("bne") & rs1.data != rs2.data) |
                                (bmop.is_uop_set("blt") & cmp_lt_sign    )  |      //////// sign mode
                                (bmop.is_uop_set("bltu")& cmp_lt_un_sign  )  |      //////// unsign
                                (bmop.is_uop_set("bge") & (~cmp_lt_sign  )) |
                                (bmop.is_uop_set("bgeu")& (~cmp_lt_un_sign));
                    //////// kill the system
                    zif(mis_predic){cd.kill();}
                    zif(jmop.is_set()){
                        zif(jmop.is_uop_set("needpc")){
                            //jdebug_need_pc = 1;
                            cd.change_pc(dec_data.pc + rs2.data);
                            rdes.data   <<= dec_data.pc + 4;
                            rdes.valid  <<= 1;
                        }zelse{
                            //jdebug_not_need_pc = 1;
                            cd.change_pc(rs1.data + rs2.data);
                            rdes.data  <<= dec_data.pc + 4;
                            rdes.valid <<= 1;
                        }
                    }
                    zif(bmop.is_set() && mis_predic){
                        //jdebug_norm_pc = 1;
                        cd.change_pc(dec_data.pc + rs3.data);
                    }
                }
                auto ld_mop = dec_data.repo.get_op("ldpc");
                zif(ld_mop.is_set()){
                    rdes.valid <<= 1;
                    zif(ld_mop.is_uop_set("needpc")){rdes.data <<= (dec_data.pc + rs2.data);}
                    zelse                        {rdes.data <<= rs2.data;}
                }

            }

            void exec_complex_alu(){
                auto op = dec_data.repo.get_op("op");
                pif(op.is_set() & (op.is_uop_set("sll") | op.is_uop_set("sra") |
                        op.is_uop_set("sr"))) {
                    cdowhile(rs2.data(0, 5) > 1){
                        zif (op.is_uop_set("sll")){ rdes.data <<= rdes.data << (rs2.data(0, 5) > 0);}
                        zif (op.is_uop_set("sra")){
                            rdes.data(0, XLEN - 1) <<= (rdes.data(0, XLEN) >> (rs2.data(0, 5) > 0));
                        }
                        zif (op.is_uop_set("sr")){ rdes.data <<= rdes.data >> (rs2.data(0, 5) > 0);}

                        zif(rs2.data(0, 5) > 1) { rs2.data <<= rs2.data - 1;}
                    }

                }
            }

            void exec_ls(){
                auto ldst = dec_data.repo.get_op("ldst");
                Reg& usign      = ldst.is_uop_set("usign");
                m_wire(pool_write_data, XLEN);
                m_wire(fin_read_data,   XLEN);

                pif(ldst.is_set()){
                    cdowhile(!read_fn) {
                        read_en = 1;
                        rdes.valid <<= ldst.is_uop_set("isload");

                        zif(ldst.is_uop_set("lsb")){
                            pool_write_data = g(_memArb.read_output(8,XLEN), rs2.data(0,8));
                            fin_read_data   = ext(_memArb.read_output(0, 8), XLEN, ~usign);

                        }
                        zif(ldst.is_uop_set("lsh")){
                            pool_write_data = g(_memArb.read_output(16,XLEN), rs2.data(0,16));
                            fin_read_data   = ext(_memArb.read_output(0, 16), XLEN, ~usign);
                        }
                        zif(ldst.is_uop_set("lsw")){
                            pool_write_data = rs2.data;
                            fin_read_data   = _memArb.read_output;
                        }
                        rdes.data <<= fin_read_data;
                        _memArb.req_write_req(~ldst.is_uop_set("isload"),read_addr, pool_write_data);
                    }
                }
            }

        };
}

#endif //KATHRYN_EXECUTE_H

//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H
#define KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H

#include "kathryn.h"
#include "alu.h"
#include "rob.h"
#include "rsvs.h"


namespace kathryn::o3{

    struct DpMod;
    struct RsvBase;
    struct BranchExec: Module{

        TagMgmt&      tag_mgmt;
        RegArch&      reg_arch;
        PipStage&     pm;
        DpMod&        disp_mod;
        Rob&          rob;
        ByPass&       bp;
        Rsvs&         rsvs;
        StoreBuf&     st_buf;
        RegSlot&      src;

        PipSimProbe* psp = nullptr;  ///DC
        m_wire(cal_addr, ADDR_LEN);
        m_wire(br_taken, 1);


        explicit BranchExec(TagMgmt& tag_mgmt,
                            RegArch& reg_arch,
                            PipStage& pm,
                            DpMod&    disp_mod,
                            Rob& rob,
                            StoreBuf& st_buf,
                            Rsvs& rsvs) :
        tag_mgmt(tag_mgmt),
        reg_arch(reg_arch),
        pm(pm),
        disp_mod(disp_mod),
        rob(rob),
        st_buf(st_buf),
        bp(reg_arch.bpp.add_by_pass_ele()),
        rsvs(rsvs),
        src(rsvs.br.exec_src){}

        void set_sim_probe(PipSimProbe* in_psp){psp = in_psp;}

        void flow() override{


            //// initialize variable
            opr& opc     = src(opcode);
            opr& src_pc   = src(pc);
            opr& src_imm  = src(imm_br);
            opr& sp_tag   = src(spec_tag);
            opr& fix_tag  = tag_mgmt.mpft.get_fix_tag(OH(sp_tag));

            opr& src_a   = src(phyIdx_1);
            opr& src_b   = src(phyIdx_2); //// take imm from br
            br_taken     = alu(src(alu_op), src_a, src_b).sl(0);

            //// assign static wire to bc
            tag_mgmt.bc.suc_tag = sp_tag;
            tag_mgmt.bc.fix_tag = fix_tag;

            //// calculate the address
            opr& next_pc = src_pc + 4;
            bp.add_src(src(rrftag), next_pc); ///// add src for bypass but the bypass trigger is in zync block

            cal_addr = next_pc;
            zif  (opc == RV32_JALR)          cal_addr = src(phyIdx_1) + src_imm;
            zelif((opc==RV32_JAL) | br_taken) cal_addr = src_pc + src_imm;

            opr& br_cond = ((opc == RV32_JALR) | (opc==RV32_JAL) | br_taken);



            pip(rsvs.br.sync){  try_init_probe(psp); ///CTRL EXEC_BRANCH

                /////// write back the data if it needed
                rob.on_write_back(src(rrftag));
                zif(src(rd_use)){
                    reg_arch.rrf.on_wback(src(rrftag), next_pc);
                    reg_arch.bpp.do_by_pass(bp);
                }

                /////// success predict
                zif (src(pred_addr) == cal_addr){ //// case suc_pred
                    on_suc_pred(fix_tag, src(spec_tag));
                }zelse{ //////// case mis_pred
                    on_mis_pred(fix_tag, src(spec_tag), cal_addr);
                }
            }
        }

        void on_mis_pred(opr& fix_tag, opr& mis_tag, opr& fix_pc);

        void on_suc_pred(opr& fix_tag, opr& suc_tag);

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H

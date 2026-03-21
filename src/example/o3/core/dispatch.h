//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H
#define KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

#include "kathryn.h"

#include "imm_gen.h"
#include "imm_br_gen.h"

#include "irsv.h"
#include "orsv.h"
#include "rsvs.h"
#include "src_opr.h"
#include "rob.h"
#include "stage_struct.h"
#include "example/o3/simulation/prober_grp.h"  ///DC


namespace kathryn::o3{

    struct DpMod: Module{
        PipStage& pm;
        Rsvs&     rsvs;
        RegArch&  reg_arch;
        TagMgmt&  tag_mgmt;
        Rob&      rob;

        RegSlot& dcd1     = pm.dc.dcd1;
        RegSlot& dcd2     = pm.dc.dcd2;
        RegSlot& dcd_share = pm.dc.dcd_shared;
                                    //// it join the two rsv together
        m_wire(aluRsvIdx2_final   , ALU_ENT_SEL + 1);  //// it is one hot index
        m_wire(mulRsvIdx2_final   , MUL_ENT_SEL);
        m_wire(branchRsvIdx2_final, BRANCH_ENT_SEL); //// it is binary index
        m_wire(lsRsvIdx2_final    , LDST_ENT_SEL);

        m_wire(dbg_isAluRsvAllocatable, 1);      ///DC
        m_wire(dbg_isBranchRsvAllocatable, 1);   ///DC
        m_wire(dbg_isRenamable, 1);              ///DC
        m_wire(dbg_imm1, DATA_LEN);              ///DC
        m_wire(dbg_imm2, DATA_LEN);              ///DC

        m_wire(dbg_isDisp1, 1); ///DC
        m_wire(dbg_isDisp2, 1); ///DC

        DpMod(PipStage& pm    , Rsvs& rsvs,
              RegArch& reg_arch, TagMgmt& tag_mgmt,
              Rob& rob):
            pm     (pm)     , rsvs   (rsvs),
            reg_arch(reg_arch), tag_mgmt(tag_mgmt),
            rob    (rob){}

        Operable& is_rsv_required(RegSlot& dcd, int RS_ENT_IDX){
            return (dcd(rs_ent) == RS_ENT_IDX) & (~dcd(invalid));
        }

        ///// check that target rsv is ready to handle them all
        Operable& is_alocatable_for_rsv(opr& busy1, opr& busy2, int RS_ENT_IDX){
            return  ((~busy1).uext(2) + (~busy2).uext(2)) >=
                (is_rsv_required(pm.dc.dcd1, RS_ENT_IDX).uext(2) +
                 is_rsv_required(pm.dc.dcd2, RS_ENT_IDX).uext(2));
        }

        void on_suc_pred(opr& suc_tag){
            dcd1(spec) <<= dcd1(spec) & (dcd1(spec_tag) != suc_tag);
            dcd2(spec) <<= dcd2(spec) & (dcd2(spec_tag) != suc_tag);
        }



        WireSlot cvtdec_instr_to_rsv(RegSlot& dcd, RegSlot& dcd_shard, opr* des_rrf , int dec_lane_idx){
            /////// dec_lane_idx start from 0
            /////// create rsv sm_rsv_i for inorder is redundant
            WireSlot des(sm_rsv_o      + sm_rsv_base +
                         sm_rsv_branch + sm_rsv_alu  +
                         sm_rsv_mul); /// sm_rsv_base + sm_rsv_oi
            /////// metadata
            des(busy)    = 1;
            des(sort_bit) = 1;
            ////////////// base
            if (dec_lane_idx == 0){
                des(pc) = dcd_shard(pc);
            }else{
                des(pc) = dcd_shard(pc) + 4;
            }
            imm_gen( dcd(inst), dcd(imm_type), des(imm));
            imm_br_gen(dcd(inst), des(imm_br));

            des(rrftag)  = reg_arch.rrf.get_req_ptr() + dec_lane_idx;
            des(rd_use)   = dcd(rd_use);
            des(alu_op)   = dcd(alu_op);
            des(spec)    = dcd(spec);
            des(spec_tag) = dcd(spec_tag);

            opr* is_des_prev_use1 = nullptr;
            opr* is_des_prev_use2 = nullptr;

            if (des_rrf != nullptr){  //// use des_rrf as the trigger to check des of previous instr
                is_des_prev_use1 = &dcd_shard(des_eq_src1);
                is_des_prev_use2 = &dcd_shard(des_eq_src2);

            }

            des(phyIdx_1, rsValid_1) = decode_src_opr(dcd, des_rrf, is_des_prev_use1,
                                1, reg_arch);
            des(phyIdx_2, rsValid_2) = decode_src_opr(dcd, des_rrf, is_des_prev_use2,
                                2, reg_arch);
            ////////////// branch
            des(opcode)    = dcd(inst)(0, 7);
            des(pred_addr) = dcd(pred_addr);

            return des;
        }

        void flow() override{

            /**
             * RSV CALCULATION
             */

            //////// alu index calculation
            auto[alu_rsv_busy , alu_rsv_idx ] = rsvs.alu1.build_free_index(nullptr   , &rsvs.alu2);
            auto[alu_rsv_busy2, alu_rsv_idx2] = rsvs.alu1.build_free_index(&alu_rsv_idx, &rsvs.alu2);
            opr& is_alu_rsv_allocatable = is_alocatable_for_rsv(alu_rsv_busy, alu_rsv_busy2, RS_ENT_ALU);
            aluRsvIdx2_final = mux(dcd1(rs_ent) == RS_ENT_ALU, alu_rsv_idx2, alu_rsv_idx);

            //////// mul index calculation
            auto[mul_rsv_busy , mul_rsv_idx ] = rsvs.mul.build_free_index(nullptr   );
            auto[mul_rsv_busy2, mul_rsv_idx2] = rsvs.mul.build_free_index(&mul_rsv_idx);
            opr& is_mul_rsv_allocatable = is_alocatable_for_rsv(mul_rsv_busy, mul_rsv_busy2, RS_ENT_MUL);
            mulRsvIdx2_final = mux(dcd1(rs_ent) == RS_ENT_MUL, mul_rsv_idx2, mul_rsv_idx);

            //////// branch index calculation
            auto[branch_rsv_busy , branch_rsv_idx ] = rsvs.br.build_free_index(nullptr);
            auto[branch_rsv_busy2, branch_rsv_idx2] = rsvs.br.build_free_index(&(branch_rsv_idx+1));
            opr& is_branch_rsv_allocatable = is_alocatable_for_rsv(branch_rsv_busy, branch_rsv_busy2, RS_ENT_BRANCH);
            branchRsvIdx2_final = mux(dcd1(rs_ent) == RS_ENT_BRANCH, branch_rsv_idx2, branch_rsv_idx);

            //////// ls index calculation
            auto[ls_rsv_busy , ls_rsv_idx ] = rsvs.ls.build_free_index(nullptr);
            auto[ls_rsv_busy2, ls_rsv_idx2] = rsvs.ls.build_free_index(&(ls_rsv_idx+1));
            opr& is_ls_rsv_allocatable = is_alocatable_for_rsv(ls_rsv_busy, ls_rsv_busy2, RS_ENT_LDST);
            lsRsvIdx2_final = mux(dcd1(rs_ent) == RS_ENT_LDST, ls_rsv_idx2, ls_rsv_idx);

            ///// rename command
            RenameCmd ren_cmd1{dcd1(rd_use)                 , reg_arch.rrf.get_req_ptr(),
                              dcd1(rd_idx)                 ,
                              dcd1(is_branch)              , dcd1(spec_tag)};
            RenameCmd ren_cmd2{dcd2(rd_use)&(~dcd2(invalid)), reg_arch.rrf.get_req_ptr()+1,
                              dcd2(rd_idx)                 ,
                              dcd2(is_branch)              , dcd2(spec_tag)};
            ///// dispatch signal
            opr& is_renamable = reg_arch.rrf.is_renamable(~dcd2(invalid));
            opr& isdispatable = is_alu_rsv_allocatable    & is_mul_rsv_allocatable &
                                is_branch_rsv_allocatable & is_ls_rsv_allocatable &
                                is_renamable;
            ////// pre assign the data to update reservation station
            WireSlot entry1(cvtdec_instr_to_rsv(dcd1, dcd_share, nullptr        , 0));
            WireSlot entry2(cvtdec_instr_to_rsv(dcd2, dcd_share, &entry1(rrftag), 1));

            ////// dbg zone
            dbg_isAluRsvAllocatable      = is_alu_rsv_allocatable;
            dbg_isBranchRsvAllocatable   = is_branch_rsv_allocatable;
            dbg_isRenamable              = is_renamable;
            dbg_imm1                     = entry1(imm);
            dbg_imm2                     = entry2(imm);

            pip(pm.sync_dp){                               init_probe(pip_prob_grp .dispatch); ///CTRL DISPATCH
                zyncc(pm.sync_rs, isdispatable){ auto_sync  init_probe(zync_prob_grp.dispatch); ///CTRL DISPATCH
                    ////////
                    reg_arch.rrf.on_rename(~dcd2(invalid));
                    opr& req_ptr = reg_arch.rrf.get_req_ptr();
                    //////// update arf
                    reg_arch.arf.on_rename(ren_cmd1, ren_cmd2);
                    ////// dcd 1 supposed to be valid all the time
                    /***
                     * dispatch entry 1
                     */
                    zif (alu_rsv_idx.sl(0)){
                        rsvs.alu2.try_write_entry(dcd1(rs_ent),
                                                alu_rsv_idx.sl(1, RS_ENT_SEL+1),
                                                entry1);
                    }zelse{
                        rsvs.alu1.try_write_entry(dcd1(rs_ent),
                                                alu_rsv_idx.sl(1, RS_ENT_SEL+1),
                                                entry1);
                    }
                    rsvs.mul.try_write_entry(dcd1(rs_ent), mul_rsv_idx   , entry1);
                    rsvs.br .try_write_entry(dcd1(rs_ent), branch_rsv_idx, entry1);
                    rsvs.ls .try_write_entry(dcd1(rs_ent), ls_rsv_idx    , entry1);
                    rob.on_dispatch(req_ptr, dcd1, dcd_share); //// acknowledge reroder buffer
                    dbg_isDisp1 = 1; ///DC
                    /***
                     * dispatch entry 2
                     */
                    zif(~dcd2(invalid)){
                        zif (aluRsvIdx2_final.sl(0)){
                            rsvs.alu2.try_write_entry(dcd2(rs_ent),
                                                    aluRsvIdx2_final(1, RS_ENT_SEL+1),
                                                    entry2);
                        }zelse{
                            rsvs.alu1.try_write_entry(dcd2(rs_ent),
                                                    aluRsvIdx2_final.sl(1, RS_ENT_SEL+1),
                                                    entry2);
                        }
                        rsvs.mul.try_write_entry(dcd2(rs_ent), mulRsvIdx2_final   , entry2);
                        rsvs.br .try_write_entry(dcd2(rs_ent), branchRsvIdx2_final, entry2);
                        rsvs.ls .try_write_entry(dcd2(rs_ent), lsRsvIdx2_final    , entry2);
                        rob.on_dispatch(req_ptr+1, dcd2, dcd_share);
                        dbg_isDisp2 = 1; ///DC
                    }
                }
            }
        }
    };
}
#endif //KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

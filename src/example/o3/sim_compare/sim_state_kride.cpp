//
// Created by tanawin on 24/12/25.
//

#include "sim_state_kride.h"


namespace kathryn::o3{

    pip_stat SimStateKride::generate_pip_state(PipSimProbe* pip_probe,
                             ZyncSimProb* zync_sim_probe){

        assert((pip_probe != nullptr) || (zync_sim_probe != nullptr));

        if (pip_probe != nullptr &&
            pip_probe->is_waiting()){ return PS_IDLE;  }

        if (zync_sim_probe == nullptr) { return PS_RUNNING; }
        if (zync_sim_probe->is_waiting()){ return PS_STALL; }
        return PS_RUNNING;
    }

    void SimStateKride::assign_ele_if_there(ull& ele, RegSlot& val, const std::string& ele_name){
        if (val.is_there_field(ele_name)){
            ele = ull(val(ele_name));
        }
    }

    void SimStateKride::assignRSV_Entry(RSV_BASE_ENTRY& entry, RegSlot& reg_slot){

        assign_ele_if_there(entry.busy   , reg_slot, busy);
        assign_ele_if_there(entry.sortbit, reg_slot, sort_bit);
        assign_ele_if_there(entry.pc     , reg_slot, pc);
        assign_ele_if_there(entry.imm    , reg_slot, imm);
        assign_ele_if_there(entry.rrftag , reg_slot, rrftag);
        assign_ele_if_there(entry.dstval , reg_slot, rd_use);
        assign_ele_if_there(entry.alu_op , reg_slot, alu_op);
        assign_ele_if_there(entry.spec_bit, reg_slot, spec);
        assign_ele_if_there(entry.spectag, reg_slot, spec_tag);

        assign_ele_if_there(entry.src1    , reg_slot, phyIdx_1);
        assign_ele_if_there(entry.src1_sel, reg_slot, rsSel_1);
        assign_ele_if_there(entry.valid1  , reg_slot, rsValid_1);
        assign_ele_if_there(entry.src2    , reg_slot, phyIdx_2);
        assign_ele_if_there(entry.src2_sel, reg_slot, rsSel_2);
        assign_ele_if_there(entry.valid2  , reg_slot, rsValid_2);

    }

    void SimStateKride::assignRSV_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& reg_slot){

        assignRSV_Entry(entry, reg_slot);
        assign_ele_if_there(entry.imm_br  , reg_slot, imm_br);
        assign_ele_if_there(entry.praddr  , reg_slot, pred_addr);
        assign_ele_if_there(entry.opcode  , reg_slot, opcode);

    }

    void SimStateKride::assignRSV_Mul(RSV_MUL_ENTRY& entry, RegSlot& reg_slot){
        assignRSV_Entry(entry, reg_slot);
        assign_ele_if_there(entry.src1_signed, reg_slot, md_req_in_signed_1);
        assign_ele_if_there(entry.src2_signed, reg_slot, md_req_in_signed_2);
        assign_ele_if_there(entry.sel_lohi   , reg_slot, md_req_out_sel);

        entry.pc      = 0; //// mul and ldst discard it
        entry.imm     = 0;
        entry.alu_op  = 0; //////  mul and ldst discard it
        entry.src1_sel = 0, entry.src2_sel = 0; ///// ldst discards it




    }


    void SimStateKride::assignEXEC_Entry (RSV_BASE_ENTRY& entry  , RegSlot& reg_slot){
        assignRSV_Entry(entry, reg_slot);
        entry.busy    = 0;
        entry.sortbit = 0;
        entry.valid1  = 0;
        entry.valid2  = 0;
    }
    void SimStateKride::assignEXEC_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& reg_slot){
        assignRSV_Branch(entry, reg_slot);
        entry.busy     = 0;
        entry.sortbit  = 0;
        entry.imm      = 0;
        entry.src1_sel = 0;
        entry.valid1   = 0;
        entry.src2_sel = 0;
        entry.valid2   = 0;

    }

    void SimStateKride::assignEXEC_Mul(RSV_MUL_ENTRY& entry, RegSlot& reg_slot){
        assignRSV_Mul(entry, reg_slot);
        entry.busy     = 0;
        entry.sortbit  = 0;
        entry.imm      = 0;
        entry.alu_op   = 0;
        entry.src1_sel = 0;
        entry.valid1   = 0;
        entry.src2_sel = 0;
        entry.valid2   = 0;
    }
    void SimStateKride::assignEXEC_LDST(RSV_BASE_ENTRY& entry, RegSlot& reg_slot){
        assignRSV_Entry(entry, reg_slot);
        entry.busy     = 0;
        entry.sortbit  = 0;
        entry.alu_op   = 0;
        entry.src1_sel = 0;
        entry.valid1   = 0;
        entry.src2_sel = 0;
        entry.valid2   = 0;
    }


    void SimStateKride::assignARF_Table(int table_idx,RegSlot& busy_slot, RegSlot& rename_slot){
        for (int reg_idx = 0; reg_idx < REG_NUM; reg_idx++){
            arf.busy  [table_idx][reg_idx] = (ull(busy_slot  (reg_idx)) != 0);
            arf.rename[table_idx][reg_idx] = ull(rename_slot(reg_idx));
        }
    }






    void SimStateKride::recruit_value(){

        //////////  bc_state
        bc_state.mis_pred  = ull(_core.tag_mgmt.bc.mis) != 0;
        bc_state.succ_pred = ull(_core.tag_mgmt.bc.suc) != 0;
        bc_prev.mis_pred  = is_last_cycle_mis_pred;
        bc_prev.succ_pred = is_last_cycle_succ;

        //// fetch
        fetch.st = generate_pip_state(&pip_prob_grp.fetch, &zync_prob_grp.fetch);
        fetch.pc = ull(_core.p_fetch.cur_pc);

        //// decode
        FetchStage&  fetch_stage = _core.pm.ft;
        decode.st       = generate_pip_state(&pip_prob_grp.decode, &zync_prob_grp.decode);
        decode.inst1    = ull(fetch_stage.raw(inst1)   );
        decode.invalid2 = ull(fetch_stage.raw(invalid2));
        decode.inst2    = ull(fetch_stage.raw(inst2)   );
        decode.pc       = ull(fetch_stage.raw(pc)      );
        decode.npc      = ull(fetch_stage.raw(npc)     );
        decode.is_genable= (ull(_core.p_dec.dbg_isGenable) != 0);

        //// dispatch
        RegSlot& dec_shared = _core.pm.dc.dcd_shared;

        dispatch.st = generate_pip_state(&pip_prob_grp.dispatch, &zync_prob_grp.dispatch);
        dispatch.pc        = ull(dec_shared(pc));
        dispatch.des_eq_src1 = ull(dec_shared(des_eq_src1));
        dispatch.des_eq_src2 = ull(dec_shared(des_eq_src2));

        DecodeStage& decode_stage = _core.pm.dc;
        for (int i = 1; i <= 2; i++){
            RegSlot& target_reg_slot = (i == 1) ? decode_stage.dcd1 : decode_stage.dcd2;
            DispInstr& disp_state   = (i == 1) ? dispatch.dp1      : dispatch.dp2;

            disp_state.invalid     = ull(target_reg_slot(invalid));
            disp_state.imm_type    = ull(target_reg_slot(imm_type));
            disp_state.alu_op       = ull(target_reg_slot(alu_op));
            disp_state.rs_ent       = ull(target_reg_slot(rs_ent));
            disp_state.is_branch    = ull(target_reg_slot(is_branch));
            disp_state.pred_addr   = ull(target_reg_slot(pred_addr));
            disp_state.spec        = ull(target_reg_slot(spec));
            disp_state.spec_tag     = ull(target_reg_slot(spec_tag));
            disp_state.rd_idx       = ull(target_reg_slot(rd_idx));
            disp_state.rd_use       = ull(target_reg_slot(rd_use));
            disp_state.rsIdx_1     = ull(target_reg_slot(rsIdx_1));
            disp_state.rsSel_1     = ull(target_reg_slot(rsSel_1));
            disp_state.rsUse_1     = ull(target_reg_slot(rsUse_1));
            disp_state.rsIdx_2     = ull(target_reg_slot(rsIdx_2));
            disp_state.rsSel_2     = ull(target_reg_slot(rsSel_2));
            disp_state.rsUse_2     = ull(target_reg_slot(rsUse_2));
        }

        dispatch.is_alu_rsv_allocatable    = ull(_core.p_disp.dbg_isAluRsvAllocatable) != 0;
        dispatch.is_branch_rsv_allocatable = ull(_core.p_disp.dbg_isBranchRsvAllocatable) != 0;
        dispatch.is_renamable            = ull(_core.p_disp.dbg_isRenamable) != 0;


        //// reservation station
        for(int idx = 0; idx < ALU_ENT_NUM; idx++){
            assignRSV_Entry(rsv_alu1[idx], _core.rsvs.alu1._table(idx));
            assignRSV_Entry(rsv_alu2[idx], _core.rsvs.alu2._table(idx));
        }
        st_issue_alu1  = generate_pip_state(nullptr, &zync_prob_grp.issue_alu1);
        idx_issue_alu1 = ull(_core.rsvs.alu1.check_idx);
        st_issue_alu2  = generate_pip_state(nullptr, &zync_prob_grp.issue_alu2);
        idx_issue_alu2 = ull(_core.rsvs.alu2.check_idx);

        for (int idx = 0; idx < MUL_ENT_NUM; idx++){
            assignRSV_Mul(rsv_mul[idx], _core.rsvs.mul._table(idx));
        }
        st_issue_mul  = generate_pip_state(nullptr, &zync_prob_grp.issue_mul);
        idx_issue_mul = ull(_core.rsvs.mul.check_idx);

        for (int idx = 0; idx < BRANCH_ENT_NUM; idx++){
            assignRSV_Branch(rsv_branch[idx], _core.rsvs.br._table(idx));
        }
        st_issue_branch  = generate_pip_state(nullptr, &zync_prob_grp.issue_branch);
        idx_issue_branch = ull(_core.rsvs.br.check_idx);

        for (int idx = 0; idx < LDST_ENT_NUM; idx++){
            assignRSV_Entry(rsv_ld_st[idx], _core.rsvs.ls._table(idx));
        }
        st_issue_ldst  = generate_pip_state(nullptr, &zync_prob_grp.issue_ld_st);
        idx_issue_ldst = ull(_core.rsvs.ls.check_idx);

        //////////////////////
        //// execute /////////
        //////////////////////

        exec_alu1.st = generate_pip_state(&pip_prob_grp.exec_alu1, nullptr);
        assignEXEC_Entry(exec_alu1.entry, _core.rsvs.alu1.exec_src);
        exec_alu2.st = generate_pip_state(&pip_prob_grp.exec_alu2, nullptr);
        assignEXEC_Entry(exec_alu2.entry, _core.rsvs.alu2.exec_src);

        exec_mul.st = generate_pip_state(&pip_prob_grp.exec_mul, nullptr);
        assignEXEC_Mul(exec_mul.entry, _core.rsvs.mul.exec_src);

        exec_branch.st = generate_pip_state(&pip_prob_grp.exec_branch, nullptr);
        assignEXEC_Branch(exec_branch.entry, _core.p_ex_bra.src);

        exec_ldst.st1 = generate_pip_state(&pip_prob_grp.exec_ld_st, &zync_prob_grp.load_store2);
        exec_ldst.st2 = generate_pip_state(&pip_prob_grp.exec_ld_st2, nullptr);
        assignEXEC_LDST(exec_ldst.entry, _core.rsvs.ls.exec_src);
        exec_ldst.eff_addr = ull(_core.p_ex_ld_st.dbg_effAddr);

        exec_ldst.rrftag    =   ull(_core.p_ex_ld_st.ls_res(rrftag));
        exec_ldst.rd_use     =   ull(_core.p_ex_ld_st.ls_res(rd_use));
        exec_ldst.spec      =   ull(_core.p_ex_ld_st.ls_res(spec));
        exec_ldst.spec_tag   =   ull(_core.p_ex_ld_st.ls_res(spec_tag));
        exec_ldst.st_buf_data =   ull(_core.p_ex_ld_st.ls_res(st_buf_data));
        exec_ldst.st_buf_hit  =   ull(_core.p_ex_ld_st.ls_res(st_buf_hit));
        exec_ldst.load_data = ull(_core.p_ex_ld_st.lss.dmem_rdata);

        //// commit
        rob.com_ptr = ull(_core.prob.com_ptr);
        rob.com1Status = ull(_core.prob.com1Status) != 0;
        rob.com2Status = ull(_core.prob.com2Status) != 0;
        for(int rrf_idx = 0; rrf_idx < RRF_NUM; rrf_idx++){
            rob.com_entries[rrf_idx].wb_fin    = ull(_core.prob._table(rrf_idx)(wb_fin).v()   );
            rob.com_entries[rrf_idx].store_bit = ull(_core.prob._table(rrf_idx)(store_bit).v());
            rob.com_entries[rrf_idx].rd_use    = ull(_core.prob._table(rrf_idx)(rd_use).v()   );
            rob.com_entries[rrf_idx].rd_idx    = ull(_core.prob._table(rrf_idx)(rd_idx).v()   );
        }
        rob.is_prev_cycle_dp1 = is_last_cycle_disp1;
        rob.is_prev_cycle_dp2 = is_last_cycle_disp2;
        rob.dp_pointer      = ull(last_dispatch_ptr);


        //// store buffer
        stbuf.fin_ptr        = ull(_core.store_buf.fin_ptr);
        stbuf.com_ptr        = ull(_core.store_buf.com_ptr);
        stbuf.ret_ptr        = ull(_core.store_buf.ret_ptr);
        stbuf.nb1           = ull(_core.store_buf.nb1.s_idx);
        stbuf.ne1           = ull(_core.store_buf.ne1.s_idx);
        stbuf.nb0           = ull(_core.store_buf.nb0.s_idx);
        stbuf.full_next      = ull(_core.store_buf.full_next) != 0;
        stbuf.empty_next     = ull(_core.store_buf.empty_next) != 0;
        for (int store_buf_idx = 0; store_buf_idx < STBUF_ENT_NUM; store_buf_idx++){
            stbuf.entries[store_buf_idx].busy     = ull(_core.store_buf._table(store_buf_idx)(busy).v());
            stbuf.entries[store_buf_idx].complete = ull(_core.store_buf._table(store_buf_idx)(complete).v());
            stbuf.entries[store_buf_idx].spec     = ull(_core.store_buf._table(store_buf_idx)(spec).v());
            stbuf.entries[store_buf_idx].spec_tag  = ull(_core.store_buf._table(store_buf_idx)(spec_tag).v());
            stbuf.entries[store_buf_idx].mem_addr = ull(_core.store_buf._table(store_buf_idx)(mem_addr).v());
            stbuf.entries[store_buf_idx].mem_data = ull(_core.store_buf.daytas.at(store_buf_idx));
        }

        /////// mpft

        Mpft& hw_mpft = _core.tag_mgmt.mpft;
        for(int sp_row_idx = 0; sp_row_idx < SPECTAG_LEN; sp_row_idx++){
            mpft.valids[sp_row_idx] = ull(hw_mpft.is_used(sp_row_idx)) != 0;
            for (int sp_col_idx = 0; sp_col_idx < SPECTAG_LEN; sp_col_idx++){
                ull row_value = ull(hw_mpft._table(sp_row_idx)(mpft_fixTag));
                ull col_value = ull(row_value >> sp_col_idx);
                mpft.fix_table[sp_row_idx][sp_col_idx] = ull((col_value & 1) != 0);
            }
        }

        /////// tag generator
        tag_gen.brdepth = ull(_core.tag_mgmt.tag_gen.brdepth);
        tag_gen.tag_reg  = ull(_core.tag_mgmt.tag_gen.tagreg);
        /////// ARF
        for(int table_idx = 0; table_idx < SPECTAG_LEN; table_idx++){
            assignARF_Table(table_idx, _core.reg_arch.arf.busy(table_idx),
                                      _core.reg_arch.arf.rename(table_idx));
        }
        assignARF_Table(SPECTAG_LEN, _core.reg_arch.arf.busy_master,
                                     _core.reg_arch.arf.rename_master);

        /////// rrf
        Table& rrf_table = _core.reg_arch.rrf.table;
        for (int idx = 0; idx < RRF_NUM; idx++){
            rrf.busy[idx] = ull(rrf_table(idx)(rrf_valid).v());
            rrf.data[idx] = ull(rrf_table(idx)(rrf_data ).v());
        }
        rrf.freenum       = ull(_core.reg_arch.rrf.freenum);
        rrf.req_ptr        = ull(_core.reg_arch.rrf.req_ptr);
        rrf.next_rrf_cycle  = ull(_core.reg_arch.rrf.next_rrf_cycle);
    }

    void SimStateKride::recruit_next_cycle(){
        is_last_cycle_mis_pred = ull(_core.tag_mgmt.bc.mis) != 0;
        is_last_cycle_succ    = ull(_core.tag_mgmt.bc.suc) != 0;
        ///////// state recorder for next cycle
        is_last_cycle_disp1   = ull(_core.p_disp.dbg_isDisp1);
        is_last_cycle_disp2   = ull(_core.p_disp.dbg_isDisp2);
        last_dispatch_ptr    = ull(_core.reg_arch.rrf.get_req_ptr());
    }

    void SimStateKride::print_slot_window(SlotWriterBase& writer){
        ////////// main printing
        SimState::print_slot_window(writer);
        ///////// rrf change printing
        TableSimProbe& tb_probe = data_struct_prob_grp.rrf;
        std::vector<SlotSimInfo64> row_change = tb_probe.detect_row_change();
        write_slot_if_table_change(writer, RPS_RRF, row_change, 256);
        data_struct_prob_grp.apply_cycle_change();
    }

    void SimStateKride::write_slot_if_table_change(
        SlotWriterBase& writer,
        REC_PIP_STAGE stage_idx,
        std::vector<SlotSimInfo64> change_rows,
        int row_lim_to_print_entire_row) const{ //// amount of row in changing if exceed, we will print only changing field
        bool not_exceed_row_lim = (change_rows.size() <= row_lim_to_print_entire_row);

        for (SlotSimInfo64& change_row : change_rows){
            writer.add_slot_val(stage_idx, "-----> row idx: " + std::to_string(change_row.row_idx));
            for (int col_idx = 0; col_idx < change_row.field_sim_infos.size(); col_idx++){
                FieldSimInfo64& field_info = change_row.field_sim_infos[col_idx];
                if (not_exceed_row_lim || field_info.prev_value != field_info.cur_value){
                    std::string field_str = field_info.name + ": " +
                                           std::to_string(field_info.prev_value) + "->" +
                                           std::to_string(field_info.cur_value);
                    writer.add_slot_val(stage_idx, field_str);
                }
            }
        }

    }



}
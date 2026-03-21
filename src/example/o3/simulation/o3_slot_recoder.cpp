//
// Created by tanawin on 14/10/25.
//

#include "o3_slot_recoder.h"

#include "prober_grp.h"

namespace kathryn::o3{



    void O3SlotRecorder::record_slot(){

        /////// write tag mgmt slot and reg arch first
        write_mpft_slot();
        write_arf_slot();
        write_rrf_slot();
        /////// write frontend
        write_fetch_slot();
        write_decode_slot();
        write_dispatch_slot();

        /////// write backend
        write_rsv_alu_slot(1, _core->rsvs.alu1);
        write_rsv_alu_slot(2, _core->rsvs.alu2);
        write_rsv_mul_slot();
        write_rsv_branch_slot();
        write_rsv_load_slot();

        /////// write backend

        ///////// rsv issue
        write_issue_alu_slot(1, _core->rsvs.alu1, zync_prob_grp.issue_alu1);
        write_issue_alu_slot(2, _core->rsvs.alu2, zync_prob_grp.issue_alu2);
        write_issue_mul_slot();
        write_issue_branch_slot();
        write_issue_ld_st_slot();

        ///////// exec issue
        write_execute_alu_slot(1, _core->rsvs.alu1, pip_prob_grp.exec_alu1);
        write_execute_alu_slot(2, _core->rsvs.alu2, pip_prob_grp.exec_alu2);
        write_execute_mul_slot();
        auto [this_cycle_mis, this_cycle_suc] =
        write_execute_branch_slot();
        write_execute_ld_st_slot();

        ///////// write commit stage
        write_commit_slot();

        ///////// write commit stage
        write_st_buf_table();

        //////// iterate the cycle
        _slotWriter->conclude_each_cycle();

        ////// update MisPred Status
        is_last_cycle_mis_pred = this_cycle_mis;
        is_last_cycle_suc_pred = this_cycle_suc;
        ////// update dispatch cycle
        last_dispatch_ptr  = ull(_core->reg_arch.rrf.get_req_ptr());
        is_last_cycle_disp1 = ull(_core->p_disp.dbg_isDisp1);
        is_last_cycle_disp2 = ull(_core->p_disp.dbg_isDisp2);
        ////// apply change on slot recorder
        data_struct_prob_grp.apply_cycle_change();
    }

    bool O3SlotRecorder::write_slot_if_pip_idle(REC_PIP_STAGE stage_idx,
                                            PipSimProbe* pip_probe){
        if (pip_probe->is_waiting()){
            _slotWriter->add_slot_val(stage_idx, "IDLE");
            return true;
        }
        return false;
    }

    bool O3SlotRecorder::write_slot_if_zync_stall(REC_PIP_STAGE stage_idx,
                                              ZyncSimProb* zync_sim_probe){
        if (zync_sim_probe->is_waiting()){
            _slotWriter->add_slot_val(stage_idx, "STALL");
            return true;
        }
        return false;
    }

    void O3SlotRecorder::write_slot_if_table_change(
        REC_PIP_STAGE stage_idx,
        std::vector<SlotSimInfo64> change_rows,
        int row_lim_to_print_entire_row) const{ //// amount of row in changing if exceed, we will print only changing field
        bool not_exceed_row_lim = (change_rows.size() <= row_lim_to_print_entire_row);

        for (SlotSimInfo64& change_row : change_rows){
            _slotWriter->add_slot_val(stage_idx, "-----> row idx: " + std::to_string(change_row.row_idx));
            for (int col_idx = 0; col_idx < change_row.field_sim_infos.size(); col_idx++){
                FieldSimInfo64& field_info = change_row.field_sim_infos[col_idx];
                if (not_exceed_row_lim || field_info.prev_value != field_info.cur_value){
                    std::string field_str = field_info.name + ": " +
                                           std::to_string(field_info.prev_value) + "->" +
                                           std::to_string(field_info.cur_value);
                    _slotWriter->add_slot_val(stage_idx, field_str);
                }
            }
        }

    }


    void O3SlotRecorder::write_mpft_slot(){

        _slotWriter->add_slot_val(RPS_MPFT, "brdepth: " + std::to_string(ull(_core->tag_mgmt.tag_gen.brdepth)));
        _slotWriter->add_slot_val(RPS_MPFT, "tag_reg: " + cvt_num2_bin_str(ull(_core->tag_mgmt.tag_gen.tagreg)));

        Table& mpft_table = _core->tag_mgmt.mpft._table;
        for (int row_idx = 0; row_idx < mpft_table.get_num_row(); row_idx++){
            RegSlot& entry = mpft_table(row_idx);
            ull sim_valid  = ull(entry(mpft_valid));
            ull sim_fixTag = ull(entry(mpft_fixTag));
            _slotWriter->add_slot_val(RPS_MPFT,
                "vl: " + std::to_string(sim_valid) + " "
                "-> " + cvt_num2_bin_str(sim_fixTag)  + " "
                "idx " + std::to_string(row_idx));
        }
        _slotWriter->add_slot_val(RPS_MPFT, "----------");
    }

    std::vector<std::string> O3SlotRecorder::get_arf_slot_val(RegSlot& busy_entry, RegSlot& rename_entry){
        std::vector<std::string> result;
        const int row_num = 4;
        const int col_num = REG_NUM/row_num;

        for (int row = 0; row < row_num; row++){
            std::string row_str;
            for (int col = 0; col < col_num; col++){
                int idx = row * col_num + col;
                ull sim_busy = ull(busy_entry(idx));
                ull sim_rename = ull(rename_entry(idx));
                row_str += sim_busy ? std::to_string(sim_rename) : "-";
                if (col < col_num - 1){
                    row_str += "|";
                }
            }
            result.push_back(row_str);
        }
        return result;
    }

    void O3SlotRecorder::write_arf_slot(){
        ////// TODO the data change may be cumbersome, we have to find the way to fix this situation
        if (is_last_cycle_mis_pred){
            _slotWriter->add_slot_val(RPS_ARF, "CHANGE FROM MISPRED");
        }else if (is_last_cycle_suc_pred){
            _slotWriter->add_slot_val(RPS_ARF, "CHANGE FROM SUCCPRED");
        }

        /////// for each spectag
        for (int table_idx = 0; table_idx < SPECTAG_LEN; table_idx++){
            std::vector<std::string> arf_table =
                get_arf_slot_val(_core->reg_arch.arf.busy(table_idx),
                              _core->reg_arch.arf.rename(table_idx));

            _slotWriter->add_slot_val(RPS_ARF, "SPECTAG: " + std::to_string(table_idx));
            for (const auto & row_idx : arf_table){
                _slotWriter->add_slot_val(RPS_ARF, row_idx);
            }
        }
        /////// for master
        std::vector<std::string> arf_table_master =
            get_arf_slot_val(_core->reg_arch.arf.busy_master,
                          _core->reg_arch.arf.rename_master);
        _slotWriter->add_slot_val(RPS_ARF, "MASTER");
        for (const auto & row_idx : arf_table_master){
            _slotWriter->add_slot_val(RPS_ARF, row_idx);
        }

    }

    void O3SlotRecorder::write_rrf_slot() const{

        ull sim_reqPtr    = ull(_core->reg_arch.rrf.req_ptr);
        ull sim_comPtr    = ull(_core->prob.com_ptr);
        ull sim_freeNum   = ull(_core->reg_arch.rrf.freenum);
        ull sim_nextCycle = ull(_core->reg_arch.rrf.next_rrf_cycle);

        std::string turn_str =  (sim_comPtr <= sim_reqPtr) ? "COM->REQ" : "REQ->COM (LB)";
        _slotWriter->add_slot_val(RPS_RRF, turn_str);
        _slotWriter->add_slot_val(RPS_RRF, "REQ PTR: " + std::to_string(sim_reqPtr));
        _slotWriter->add_slot_val(RPS_RRF, "COM PTR: " + std::to_string(sim_comPtr));
        _slotWriter->add_slot_val(RPS_RRF, "FREE SIZE" + std::to_string(sim_freeNum));
        _slotWriter->add_slot_val(RPS_RRF, "NEXT CY: " + std::to_string(sim_nextCycle));

        _slotWriter->add_slot_val(RPS_RRF, "--------");
        TableSimProbe& tb_probe = data_struct_prob_grp.rrf;
        std::vector<SlotSimInfo64> row_change = tb_probe.detect_row_change();
        write_slot_if_table_change(RPS_RRF, row_change, 256);
    }

    void O3SlotRecorder::write_fetch_slot(){
            ////////// write pipe status
            bool idle = write_slot_if_pip_idle(RPS_FETCH, &pip_prob_grp.fetch);
            if (idle) {return;}
            write_slot_if_zync_stall(RPS_FETCH, &zync_prob_grp.fetch);
            ////////// write pipe detail
            FetchMod& fet_mod = _core->p_fetch;

            ///// get the pc
            _slotWriter->add_slot_val(RPS_FETCH, "PC");
            _slotWriter->add_slot_val(RPS_FETCH, cvt_num2_hex_str(ull(fet_mod.cur_pc)));

            _slotWriter->add_slot_val(RPS_FETCH, cvt_num2_hex_str(ull(fet_mod.pm.ft.i_mem0)));
            _slotWriter->add_slot_val(RPS_FETCH, cvt_num2_hex_str(ull(fet_mod.pm.ft.i_mem1)));
            _slotWriter->add_slot_val(RPS_FETCH, cvt_num2_hex_str(ull(fet_mod.pm.ft.i_mem2)));
            _slotWriter->add_slot_val(RPS_FETCH, cvt_num2_hex_str(ull(fet_mod.pm.ft.i_mem3)));
    }

    void O3SlotRecorder::write_decode_slot(){
        ////////// write pipe status
        bool idle = write_slot_if_pip_idle(RPS_DECODE, &pip_prob_grp.decode);
        if (idle) {return;}
        write_slot_if_zync_stall(RPS_DECODE, &zync_prob_grp.decode);
        _slotWriter->add_slot_val(RPS_DECODE, "gennable "+ std::to_string(ull(_core->p_dec.dbg_isGenable)));
        ////////// write pipe detail
        FetchStage&  fetch_stage = _ps->ft;


        ull sim_invalid1   = 0;
        ull sim_instr1     = ull(fetch_stage.raw(inst1));
        ull sim_invalid2   = ull(fetch_stage.raw(invalid2));
        ull sim_instr2     = ull(fetch_stage.raw(inst2));
        ull sim_pc         = ull(fetch_stage.raw(pc));
        ull sim_npc        = ull(fetch_stage.raw(npc));

        _slotWriter->add_slot_val(RPS_DECODE, "PC" + cvt_num2_hex_str(ull(sim_pc)));
        _slotWriter->add_slot_val(RPS_DECODE, "NPC" + cvt_num2_hex_str(ull(sim_npc)));

        for (int i = 1; i <= 2; i++){
            ull sim_invalid = (i == 1) ? sim_invalid1 : sim_invalid2;
            ull sim_instr = (i == 1) ? sim_instr1 : sim_instr2;

            if (sim_invalid){
                _slotWriter->add_slot_val(RPS_DECODE, "not_valid");
            }else{
                std::string dec_str = "valid OP: ";
                dec_str += translate_opcode(sim_instr);
                _slotWriter->add_slot_val(RPS_DECODE, dec_str);
            }
        }
    }

    void O3SlotRecorder::write_dispatch_slot(){
        ////////// write pipe status
        bool idle = write_slot_if_pip_idle(RPS_DISPATCH, &pip_prob_grp.dispatch);
        if (idle) {return;}
        write_slot_if_zync_stall(RPS_DISPATCH, &zync_prob_grp.dispatch);
        ////////// write pipe detail
        DecodeStage& decode_stage = _ps->dc;

        RegSlot& dec_shared = _ps->dc.dcd_shared;
        ull sim_shared_pc        = ull(dec_shared(pc));
        ull sim_shared_desEqSrc1 = ull(dec_shared(des_eq_src1));
        ull sim_shared_desEqSrc2 = ull(dec_shared(des_eq_src2));
        _slotWriter->add_slot_val(RPS_DISPATCH, "alu_rsv_able: " + std::to_string(ull(_core->p_disp.dbg_isAluRsvAllocatable)));
        _slotWriter->add_slot_val(RPS_DISPATCH, "br_rsv_able: " + std::to_string(ull(_core->p_disp.dbg_isBranchRsvAllocatable)));
        _slotWriter->add_slot_val(RPS_DISPATCH, "is_renam: " + std::to_string(ull(_core->p_disp.dbg_isRenamable)));
        _slotWriter->add_slot_val(RPS_DISPATCH, "PC: " + cvt_num2_hex_str(sim_shared_pc));
        std::string internal_dep = str("S1EqDes: ") + (sim_shared_desEqSrc1? "1 " : "0 ") +
                                  str("S2EqDes: ") + (sim_shared_desEqSrc2? "1 " : "0 ");

        _slotWriter->add_slot_val(RPS_DISPATCH, internal_dep);

        for (int i = 1; i <= 2; i++){
            _slotWriter->add_slot_val(RPS_DISPATCH, "------- DIP " + std::to_string(i) + "-------");
            RegSlot& target_reg_slot = (i == 1) ? decode_stage.dcd1: decode_stage.dcd2;

            ull sim_invalid   = ull(target_reg_slot(invalid));
            ull sim_immType   = ull(target_reg_slot(imm_type));
            ull sim_aluOp     = ull(target_reg_slot(alu_op));
            ull sim_rsEnt     = ull(target_reg_slot(rs_ent));
            ull sim_isBranch  = ull(target_reg_slot(is_branch));
            ull sim_pred_addr = ull(target_reg_slot(pred_addr));
            ull sim_spec      = ull(target_reg_slot(spec));
            ull sim_specTag   = ull(target_reg_slot(spec_tag));
            ull sim_illLegal  = ull(target_reg_slot(ill_legal));
            ull sim_rdIdx     = ull(target_reg_slot(rd_idx));
            ull sim_rdUse     = ull(target_reg_slot(rd_use));
            ull sim_rsIdx_1   = ull(target_reg_slot(rsIdx_1));
            ull sim_rsSel_1   = ull(target_reg_slot(rsSel_1));
            ull sim_rsUse_1   = ull(target_reg_slot(rsUse_1));
            ull sim_rsIdx_2   = ull(target_reg_slot(rsIdx_2));
            ull sim_rsSel_2   = ull(target_reg_slot(rsSel_2));
            ull sim_rsUse_2   = ull(target_reg_slot(rsUse_2));

            if (sim_invalid){
                _slotWriter->add_slot_val(RPS_DISPATCH, "not_valid");
            }else{
                std::map<ull, std::string> imm_type_map = { {0b00, "IMM_I"}, {0b01, "IMM_S"}, {0b10, "IMM_U"}, {0b11, "IMM_J"}};
                std::map<ull, std::string> rsv_type_map = {
                    {RS_ENT_ALU    , "ALU"}, {RS_ENT_BRANCH , "BRANCH"}, {RS_ENT_JAL    , "JAL"}, {RS_ENT_JALR   , "JALR"},
                    {RS_ENT_MUL    , "MUL"}, {RS_ENT_DIV    , "DIV"}, {RS_ENT_LDST   , "LDST"}
                };
                std::string imm_type_str = (imm_type_map.find(sim_immType) != imm_type_map.end()) ? imm_type_map[sim_immType] : "UNKNOWN";
                std::string rs_type_str = (rsv_type_map.find(sim_rsEnt) != rsv_type_map.end()) ? rsv_type_map[sim_rsEnt] : "UNKNOWN";
                _slotWriter->add_slot_val(RPS_DISPATCH, "RS: " + rs_type_str);
                _slotWriter->add_slot_val(RPS_DISPATCH, imm_type_str);
                _slotWriter->add_slot_val(RPS_DISPATCH, "ALU: " + translate_alu_op(sim_aluOp));

                _slotWriter->add_slot_val(RPS_DISPATCH, "is_br: " + std::to_string(sim_isBranch) +
                                                      "/is_sp: " + std::to_string(sim_spec) +
                                                      "/sp_tag: " + cvt_num2_bin_str(sim_specTag));
                _slotWriter->add_slot_val(RPS_DISPATCH, "nextPc_" + cvt_num2_hex_str(sim_pred_addr));

                std::string rd_usage = sim_rdUse ? "(USE)" : "(UNUSED)";
                _slotWriter->add_slot_val(RPS_DISPATCH, "RD: "+ rd_usage + " /ArchIdx: " +  std::to_string(sim_rdIdx));

                std::map<ull, std::string> src_a_sel_map = { {0, "RS1"}, {1, "PC"}, {2, "ZERO"}};

                std::map<ull, std::string> src_b_sel_map = { {0, "RS2"}, {1, "IMM"}, {2, "FOUR"}, {3, "ZERO"}};
                std::string r1Usage = sim_rsUse_1 ? "(USE)" : "(UNUSED)";
                std::string sel_str = (src_a_sel_map.find(sim_rsSel_1) != src_a_sel_map.end())
                                         ? src_a_sel_map[sim_rsSel_1]
                                         : "UNKNOWN";
                _slotWriter->add_slot_val(RPS_DISPATCH, "RS1: " + r1Usage + " /IDX:" + std::to_string(sim_rsIdx_1) +
                                        " /SEL:" + sel_str);

                std::string r2Usage = sim_rsUse_2 ? "(USE)" : "(UNUSED)";
                std::string sel_str2 = (src_b_sel_map.find(sim_rsSel_2) != src_b_sel_map.end())
                                         ? src_b_sel_map[sim_rsSel_2]
                                         : "UNKNOWN";
                _slotWriter->add_slot_val(RPS_DISPATCH, "RS2: " + r2Usage + " /IDX:" + std::to_string(sim_rsIdx_2) +
                                        " /SEL:" + sel_str2);
            }
        }
    }

    /**
     *
     * RSV writing section
     *
     */

    std::pair<bool, std::vector<std::string>> O3SlotRecorder::write_rsv_slot(RegSlot& entry){

        ///// entry identifier
        ull sim_busy      = ull(entry(busy));

        ull sim_pc        = ull(entry(pc));
        ull sim_rrftag    = ull(entry(rrftag));
        //ull sim_rdUse     = ull(entry(rd_use));
        ull sim_spec      = ull(entry(spec));
        ull sim_specTag   = ull(entry(spec_tag));
        ull sim_phyIdx_1  = ull(entry(phyIdx_1));
        ///ull sim_rsSel_1   = ull(entry(rsSel_1));
        ull sim_rsValid_1 = ull(entry(rsValid_1));
        ull sim_phyIdx_2  = ull(entry(phyIdx_2));
        ///ull sim_rsSel_2   = ull(entry(rsSel_2));
        ull sim_rsValid_2 = ull(entry(rsValid_2));

        if (!sim_busy){
            return{false, {}};
        }
        std::vector<std::string> results;
        //////// sort bit/ pc/ register
        std::string result0;

        if (entry.is_there_field(sort_bit)){
            ull sim_sortBit = ull(entry(sort_bit));
            result0 += "sb:" + std::string(sim_sortBit ? "1" : "0") + " ";
        }

        result0 += "pc:" + cvt_num2_hex_str(sim_pc) + " ";
        result0 += "pd:" + std::to_string(sim_rrftag) + " ";


        bool ready = sim_rsValid_1 && sim_rsValid_2;

        if (ready){
            result0 += "READY!";
        }else{
            result0 += "W:";
            if (!sim_rsValid_1){
                result0 += std::to_string(sim_phyIdx_1);
            }
            if (!sim_rsValid_2){
                result0 += "-";
                result0 += std::to_string(sim_phyIdx_2);
            }
        }

        results.push_back(result0);

        ////// result 2 speculative meta data
        std::string result1;
        result1 += "spec: " + std::to_string(sim_spec) + " spt:" + cvt_num2_bin_str(sim_specTag);

        results.push_back(result1);

        ////// result 3 multiplication

        std::string result3;

        if (entry.is_there_field(md_req_in_signed_1)){
            std::string r1s = ull(entry(md_req_in_signed_1)) ? "s" : "u";
            std::string r2s = ull(entry(md_req_in_signed_2)) ? "s" : "u";
            std::string hl  = ull(entry(md_req_out_sel))     ? "h" : "l";

            result3 += "mul_req: 1:" + r1s + " "
                             + "2:" + r2s + " "
                             + "sl:";
            results.push_back(result3);
        }

        return {true, results};
    }


    void O3SlotRecorder::write_rsv_basic_slot(Table& table){

        for (int row_idx = 0; row_idx < table.get_num_row(); row_idx++){
            RegSlot& entry = table(row_idx);
            bool is_used = false;
            std::vector<std::string> results;
            std::tie(is_used, results) = write_rsv_slot(entry);
            if (is_used){
                bool is_first = true;
                for (std::string& result : results){
                    std::string prefix = is_first ? (std::to_string(row_idx) + "] ") : "";
                    _slotWriter->add_slot_val(RPS_RSV, prefix + result);
                    is_first = false;
                }

            }
        }

    }


    void O3SlotRecorder::write_rsv_alu_slot(int idx, ORsv& orsv){
        /////// write for alu rsv
        _slotWriter->add_slot_val(RPS_RSV, "ALU RSV " + std::to_string(idx));
        write_rsv_basic_slot(orsv._table);
        _slotWriter->add_slot_val(RPS_RSV, "----------");
    }

    void O3SlotRecorder::write_rsv_mul_slot(){
        ORsv& mul_rsv = _core->rsvs.mul;
        _slotWriter->add_slot_val(RPS_RSV, "MUL RSV ");
        write_rsv_basic_slot(mul_rsv._table);
        _slotWriter->add_slot_val(RPS_RSV, "----------");
    }


    void O3SlotRecorder::write_rsv_branch_slot(){
        /////// write for branch rsv
        IRsv& branch_i_rsv = _core->rsvs.br;
        _slotWriter->add_slot_val(RPS_RSV, "BRANCH RSV");
        _slotWriter->add_slot_val(RPS_RSV, "alloc_ptr : " + std::to_string(ull(branch_i_rsv.alloc_ptr)));
        write_rsv_basic_slot(branch_i_rsv._table);
        _slotWriter->add_slot_val(RPS_RSV, "----------");
    }

    void O3SlotRecorder::write_rsv_load_slot(){
        IRsv& ldst_i_rsv = _core->rsvs.ls;
        _slotWriter->add_slot_val(RPS_RSV, "LDST RSV");
        _slotWriter->add_slot_val(RPS_RSV, "alloc_ptr : " + std::to_string(ull(ldst_i_rsv.alloc_ptr)));
        write_rsv_basic_slot(ldst_i_rsv._table);
        _slotWriter->add_slot_val(RPS_RSV, "----------");
    }

    /**
     * Load/Store Buffer
     *
     */
    std::pair<bool, std::vector<std::string>>
    O3SlotRecorder::write_st_buf_slot(RegSlot& entry){

        ull sim_busy     = ull(entry(busy));
        ull sim_complete = ull(entry(complete));
        ull sim_spec     = ull(entry(spec));
        ull sim_specTag  = ull(entry(spec_tag));
        ull sim_mem_addr = ull(entry(mem_addr));

        std::vector<std::string> results;
        std::string result = "cpt: " + std::to_string(sim_complete) +
            " /sp:" + std::to_string(sim_spec) +
            " /spt:" + cvt_num2_bin_str(sim_specTag);
        results.push_back(result);
        results.push_back("addr: " + cvt_num2_hex_str(sim_mem_addr));

        return {static_cast<bool>(sim_busy), results};
    }

    void O3SlotRecorder::write_st_buf_table(){
        _slotWriter->add_slot_val(RPS_STBUF, "fin_ptr: " + std::to_string(ull(_core->store_buf.fin_ptr)));
        _slotWriter->add_slot_val(RPS_STBUF, "com_ptr: " + std::to_string(ull(_core->store_buf.com_ptr)));
        _slotWriter->add_slot_val(RPS_STBUF, "ret_ptr: " + std::to_string(ull(_core->store_buf.ret_ptr)));
        Table& table = _core->store_buf._table;
        for (int row_idx = 0; row_idx < table.get_num_row(); row_idx++){
            RegSlot& entry = table(row_idx);
            bool is_used = false;
            std::vector<std::string> results;
            std::tie(is_used, results) = write_st_buf_slot(entry);
            if (is_used){
                bool is_first = true;
                for (std::string& result : results){
                    std::string prefix = is_first ? (std::to_string(row_idx) + "] ") : "";
                    _slotWriter->add_slot_val(RPS_STBUF, prefix + result);
                    is_first = false;
                }

            }
        }
    }




    /**
     *
     * ISSUE writing section
     *
     */

    void O3SlotRecorder::write_issue_alu_slot(int idx, ORsv& orsv, ZyncSimProb& z_issue_probe){
        /////// write for alu issue
        _slotWriter->add_slot_val(RPS_ISSUE, "ALU ISSUE " + std::to_string(idx));
        bool is_stall = write_slot_if_zync_stall(RPS_ISSUE, &z_issue_probe);
        if (!is_stall){
            _slotWriter->add_slot_val(RPS_ISSUE, "issue Enty: " + cvt_num2_bin_str(ull(orsv.check_idx)));
        }
        _slotWriter->add_slot_val(RPS_ISSUE, "slot_ready: " + std::to_string(ull(orsv.dbg_isSlotReady)));
        _slotWriter->add_slot_val(RPS_ISSUE, "----------");
        
    }

    void O3SlotRecorder::write_issue_mul_slot(){
        ORsv& mul_rsv = _core->rsvs.mul;
        _slotWriter->add_slot_val(RPS_ISSUE, "MUL ISSUE");
        bool is_stall = write_slot_if_zync_stall(RPS_ISSUE, &zync_prob_grp.issue_mul);
        if (!is_stall){
            _slotWriter->add_slot_val(RPS_ISSUE, "issue Enty: " + std::to_string(ull(mul_rsv.check_idx)));
        }
    }

    void O3SlotRecorder::write_issue_branch_slot(){
        /////// write for branch issue
        _slotWriter->add_slot_val(RPS_ISSUE, "BRANCH ISSUE");
        bool is_stall = write_slot_if_zync_stall(RPS_ISSUE, &zync_prob_grp.issue_branch);
        if (!is_stall){
            IRsv& branch_i_rsv = _core->rsvs.br;
            _slotWriter->add_slot_val(RPS_ISSUE, "issue Enty: " + std::to_string(ull(branch_i_rsv.check_idx)));
        }
        _slotWriter->add_slot_val(RPS_ISSUE, "----------");
    }

    void O3SlotRecorder::write_issue_ld_st_slot(){
        /////// write for branch issue
        _slotWriter->add_slot_val(RPS_ISSUE, "LDST ISSUE");
        bool is_stall = write_slot_if_zync_stall(RPS_ISSUE, &zync_prob_grp.issue_ld_st);
        if (!is_stall){
            IRsv& ld_st_i_rsv = _core->rsvs.ls;
            _slotWriter->add_slot_val(RPS_ISSUE, "issue Enty: " + std::to_string(ull(ld_st_i_rsv.check_idx)));
        }
        _slotWriter->add_slot_val(RPS_ISSUE, "----------");
    }


    void O3SlotRecorder::write_execute_basic(RegSlot& src){
        ull sim_pc        = ull(src(pc));
        ull sim_rrftag    = ull(src(rrftag));
        ull sim_rdUse     = ull(src(rd_use));
        ull sim_aluOp     = ull(src(alu_op));
        ull sim_spec      = ull(src(spec));
        ull sim_specTag   = ull(src(spec_tag));
        ull sim_phyIdx_1  = ull(src(phyIdx_1));
        ull sim_rsSel_1   = ull(src(rsSel_1));
        ull sim_rsValid_1 = ull(src(rsValid_1));
        ull sim_phyIdx_2  = ull(src(phyIdx_2));
        ull sim_rsSel_2   = ull(src(rsSel_2));
        ull sim_rsValid_2 = ull(src(rsValid_2));

        std::map<ull, std::string> alu_op_map = {
            { 0, "ADD"} ,{ 1, "SLL"},{ 4, "XOR"},
            { 6, "OR"}  ,{ 7, "AND"},{ 5, "SRL"},
            { 8, "SEQ"} ,{ 9, "SNE"},{10, "SUB"},
            {11, "SRA"} ,{12, "SLT"},{13, "SGE"},
            {14, "SLTU"},{15, "SGEU"}
        };
        
        _slotWriter->add_slot_val(RPS_EXECUTE, "PC: " + cvt_num2_hex_str(sim_pc));

        if (src.is_there_field(imm)){
            ull sim_imm = ull(src(imm));
            _slotWriter->add_slot_val(RPS_EXECUTE, "IMM: " + cvt_num2_hex_str(sim_imm));
        }else if (src.is_there_field(imm_br)){
            ull sim_imm = ull(src(imm_br));
            _slotWriter->add_slot_val(RPS_EXECUTE, "IMM_BR: " + cvt_num2_hex_str(sim_imm));
        }

        _slotWriter->add_slot_val(RPS_EXECUTE, "ALU Op: " + translate_alu_op(sim_aluOp) +
                                             "/Spec: " + std::to_string(sim_spec) +
                                             "/SpecTag: " + cvt_num2_bin_str(sim_specTag));
        std::string sim_isRdUsed = sim_rdUse ? "(USE)" : "(UNUSED)";
        if (sim_rdUse){
            RegSlot&  target_reg_slot = _core->prob._table(static_cast<int>(sim_rrftag));
            ull sim_rdIdx    = ull(target_reg_slot(rd_idx));

            _slotWriter->add_slot_val(RPS_EXECUTE, "RD phy: " + std::to_string(sim_rrftag) + " arch: " + std::to_string(sim_rdIdx));
        }else{
            _slotWriter->add_slot_val(RPS_EXECUTE, "RD phy: " + std::to_string(sim_rrftag) + " arch(UNUSED)");
        }



        std::map<ull, std::string> src_a_sel_map = {{0, "RS1"}, {1, "PC"}, {2, "ZERO"}};

        std::map<ull, std::string> src_b_sel_map = {{0, "RS2"}, {1, "IMM"}, {2, "FOUR"}, {3, "ZERO"}};
        /////////// rs1
        std::string sim_rs1Valid = sim_rsValid_1 ? "(valid)" : "(false)";
        std::string sel_str = (src_a_sel_map.find(sim_rsSel_1) != src_a_sel_map.end())
                                         ? src_a_sel_map[sim_rsSel_1]
                                         : "UNKNOWN";
        _slotWriter->add_slot_val(RPS_EXECUTE, "RS1" + sim_rs1Valid +
                                             " /Data: " + std::to_string(sim_phyIdx_1) +
                                             " /Sel: " + sel_str);
        /////////// rs2
        std::string sim_rs2Valid = sim_rsValid_2 ? "(valid)" : "(false)";
        std::string sel_str2 = (src_b_sel_map.find(sim_rsSel_2) != src_b_sel_map.end())
                                         ? src_b_sel_map[sim_rsSel_2]
                                         : "UNKNOWN";
        _slotWriter->add_slot_val(RPS_EXECUTE, "RS2" + sim_rs2Valid +
                                " /Data: " + std::to_string(sim_phyIdx_2) +
                                " /Sel: " + sel_str2);
        /////////// mul
        if (src.is_there_field(md_req_in_signed_1)){
            std::string r1s = ull(src(md_req_in_signed_1)) ? "s" : "u";
            std::string r2s = ull(src(md_req_in_signed_2)) ? "s" : "u";
            std::string hl  = ull(src(md_req_out_sel))     ? "h" : "l";

            std::string result3 = "mul_req: 1:" + r1s + " "
                                         + "2:" + r2s + " "
                                         + "sl:";
            _slotWriter->add_slot_val(RPS_EXECUTE, result3);
        }
    }

    void O3SlotRecorder::write_execute_ldst_basic(RegSlot& src){

        ull sim_rrftag    = ull(src(rrftag));
        ull sim_rdUse     = ull(src(rd_use));
        ull sim_spec      = ull(src(spec));
        ull sim_specTag   = ull(src(spec_tag));
        ull sim_stBufData = ull(src(st_buf_data));
        ull sim_stBufHit  = ull(src(st_buf_hit));

        _slotWriter->add_slot_val(RPS_EXECUTE, "/Spec: " + std::to_string(sim_spec) +
                                             "/SpecTag: " + cvt_num2_bin_str(sim_specTag));
        std::string sim_isRdUsed = sim_rdUse ? "(USE)" : "(UNUSED)";
        if (sim_rdUse){
            RegSlot&  target_reg_slot = _core->prob._table(static_cast<int>(sim_rrftag));
            ull sim_rdIdx    = ull(target_reg_slot(rd_idx));

            _slotWriter->add_slot_val(RPS_EXECUTE, "RD phy: " + std::to_string(sim_rrftag) + " arch: " + std::to_string(sim_rdIdx));
        }else{
            _slotWriter->add_slot_val(RPS_EXECUTE, "RD phy: " + std::to_string(sim_rrftag) + " arch(UNUSED)");
        }

        _slotWriter->add_slot_val(RPS_EXECUTE, "st_buf_hit: " + std::to_string(sim_stBufHit) +
                                             "/st_buf_data: " + std::to_string(sim_stBufData));

    }



    void O3SlotRecorder::write_execute_alu_slot(int idx, ORsv& orsv, PipSimProbe& p_exec_probe){

        _slotWriter->add_slot_val(RPS_EXECUTE, "ALU EXEC " + std::to_string(idx));
        bool alu_idle = write_slot_if_pip_idle(RPS_EXECUTE, &p_exec_probe);
        if (alu_idle){return;}

        write_execute_basic(orsv.exec_src);
        _slotWriter->add_slot_val(RPS_EXECUTE, "----------");
    }

    void O3SlotRecorder::write_execute_mul_slot(){
        ORsv& mul_rsv = _core->rsvs.mul;
        _slotWriter->add_slot_val(RPS_EXECUTE, "MUL EXEC");
        bool mul_idle = write_slot_if_pip_idle(RPS_EXECUTE, &pip_prob_grp.exec_mul);
        if (mul_idle){return;}
        write_execute_basic(mul_rsv.exec_src);
        _slotWriter->add_slot_val(RPS_EXECUTE, "----------");
    }

    std::pair<bool, bool> O3SlotRecorder::write_execute_branch_slot(){
        _slotWriter->add_slot_val(RPS_EXECUTE, "BRANCH EXEC");
        bool branch_idle = write_slot_if_pip_idle(RPS_EXECUTE, &pip_prob_grp.exec_branch);
        if (branch_idle){return {false, false};}

        bool is_this_cycle_mis_pred = false;
        bool is_this_cycle_succ = false;

        _slotWriter->add_slot_val(RPS_EXECUTE, "---BRANCH STATUS");
        if (ull(_core->tag_mgmt.bc.mis)){
            _slotWriter->add_slot_val(RPS_EXECUTE, "MISPREDICTED");
            is_this_cycle_mis_pred = true;
        }
        if (ull(_core->tag_mgmt.bc.suc)){
            _slotWriter->add_slot_val(RPS_EXECUTE, "SUC PREDICTED");
            is_last_cycle_mis_pred = true;

        }
        IRsv& branch_i_rsv = _core->rsvs.br;
        _slotWriter->add_slot_val(RPS_EXECUTE, "OP: " + translate_opcode(ull(branch_i_rsv.exec_src(opcode))));
        _slotWriter->add_slot_val(RPS_EXECUTE, "---");
        write_execute_basic(branch_i_rsv.exec_src);

        return {is_this_cycle_mis_pred, is_this_cycle_succ};
    }

    void O3SlotRecorder::write_execute_ld_st_slot(){
        IRsv& ldst_i_rsv = _core->rsvs.ls;
        ////////////////// load store 1
        _slotWriter->add_slot_val(RPS_EXECUTE, "LDST EXEC 1");
        bool ld_st_idle = write_slot_if_pip_idle(RPS_EXECUTE, &pip_prob_grp.exec_ld_st);
        if (!ld_st_idle){
            write_execute_basic(ldst_i_rsv.exec_src);
        }
        _slotWriter->add_slot_val(RPS_EXECUTE, "----------");

        ////////////////// load store 2
        _slotWriter->add_slot_val(RPS_EXECUTE, "LDST EXEC 2");
        bool ld_st_idle2 = write_slot_if_pip_idle(RPS_EXECUTE, &pip_prob_grp.exec_ld_st2);
        if (ld_st_idle2){return;}
        write_execute_ldst_basic(_core->pm.ld_st.ls_res);
    }

    std::vector<std::string> O3SlotRecorder::write_rob_slot(ull rob_idx){

        std::vector<std::string> result;

        RegSlot&  target_reg_slot = _core->prob._table(static_cast<int>(rob_idx));

        ull sim_wbFin    = ull(target_reg_slot(wb_fin));
        ull sim_isBranch = ull(target_reg_slot(is_branch));
        ull sim_rdUse    = ull(target_reg_slot(rd_use));
        ull sim_rdIdx    = ull(target_reg_slot(rd_idx));

        ull sim_storeBit = ull(target_reg_slot(store_bit));

        ull sim_pc       = ull(target_reg_slot(pc));
        // ull sim_jumpAddr = ull(target_reg_slot(jump_addr));
        // ull sim_jumpCond = ull(target_reg_slot(jump_cond));


        //////// basic string for rob entry
        std::string entry_str = "E:" + std::to_string(rob_idx);
        entry_str += "/fin:" + std::to_string(sim_wbFin);

        entry_str += "/rd:";
        if (sim_rdUse){
            entry_str += std::to_string(sim_rdIdx);
        }else{
            entry_str += "-";
        }

        if (sim_storeBit){
            entry_str += "/Store";
        }
        result.push_back(entry_str);

        //////// branch info add in case it is branch
        // if (sim_isBranch){
        //     result.push_back("/br: c  "    + std::to_string(sim_jumpCond));
        //     result.push_back("/br: pc "  + cvt_num2_hex_str(sim_pc));
        //     result.push_back("/br: to "  + cvt_num2_hex_str(sim_jumpAddr));
        // }
        return result;

    }


    void O3SlotRecorder::write_commit_slot(){


        _slotWriter->add_slot_val(RPS_COMMIT, "----- dispatched (prev_cycle)");
        int amt_disp = static_cast<int>(is_last_cycle_disp1 + is_last_cycle_disp2);
        int rob_size = 1 << _core->reg_arch.rrf.get_req_ptr().get_operable_slice().get_size();
        _slotWriter->add_slot_val(RPS_COMMIT, "disp_ptr: " + std::to_string(ull(_core->reg_arch.rrf.req_ptr)));
        _slotWriter->add_slot_val(RPS_COMMIT, "disp_amt: " + std::to_string(ull(amt_disp)));
        for (int i = 0; i < amt_disp; i++){
            _slotWriter->add_slot_vals(RPS_COMMIT, write_rob_slot((last_dispatch_ptr + i)%rob_size));
        }


        _slotWriter->add_slot_val(RPS_COMMIT, "----- committing");
        int amt_commit = static_cast<int>(ull(_core->prob.com1Status) + ull(_core->prob.com2Status));
        _slotWriter->add_slot_val(RPS_COMMIT, "cm_ptr: " + std::to_string(ull(_core->prob.com_ptr)));
        _slotWriter->add_slot_val(RPS_COMMIT, "cm_amt: " + std::to_string(amt_commit));

        for (int i = 0; i < amt_commit; i++){
            _slotWriter->add_slot_vals(RPS_COMMIT, write_rob_slot((ull(_core->prob.com_ptr) + i) % rob_size));
        }

        // _slotWriter->add_slot_val(RPS_COMMIT, "----- changing");
        // TableSimProbe& tb_probe = data_struct_prob_grp.commit;
        // std::vector<SlotSimInfo64> row_change = tb_probe.detect_row_change();
        // write_slot_if_table_change(RPS_COMMIT, row_change, 256);

    }

    std::string O3SlotRecorder::translate_opcode(ull raw_instr){
        std::map<ull, std::string> dec_map = {
            {0b00'000'11, "LOAD"   },
            {0b01'000'11, "STORE"  },
            {0b00'011'11, "MISCMEM"},
            {0b11'000'11, "BRANCH" },
            {0b11'001'11, "JALR"   },
            {0b11'011'11, "JAL"    },
            {0b00'100'11, "OP_IMM" },
            {0b01'100'11, "OP"     },
            {0b00'101'11, "AUIPC"  },
            {0b01'101'11, "LUI"    },
            {0b11'100'11, "SYSTEM" },
        };

        ull op_mask_bit = (1 << 7) - 1;
        ull op = raw_instr & op_mask_bit;
        return (dec_map.find(op) != dec_map.end()) ? dec_map[op] : "UNKNOWN";
    }

    std::string O3SlotRecorder::translate_alu_op(ull alu_op_idx){
        std::map<ull, std::string> alu_op_map = {
            { 0, "ADD"},
            { 1, "SLL"},
            { 4, "XOR"},
            { 6, "OR"},
            { 7, "AND"},
            { 5, "SRL"},
            { 8, "SEQ"},
            { 9, "SNE"},
            {10, "SUB"},
            {11, "SRA"},
            {12, "SLT"},
            {13, "SGE"},
            {14, "SLTU"},
            {15, "SGEU"}
        };
        return (alu_op_map.find(alu_op_idx) != alu_op_map.end()) ? alu_op_map[alu_op_idx] : "UNKNOWN";
    }

}

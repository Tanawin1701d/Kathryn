//
// Created by tanawin on 24/12/25.
//

#include "sim_state.h"

namespace kathryn::o3{


    /**
     * shared functionality
     *
     */

    std::string lookup_imm_type(ull key){
        static const std::map<ull, std::string> imm_type_map = {
            {0b00, "IMM_I"}, {0b01, "IMM_S"},
            {0b10, "IMM_U"}, {0b11, "IMM_J"}
        };
        auto it = imm_type_map.find(key);
        return (it != imm_type_map.end()) ? it->second : "UNKNOWN";
    }

    std::string lookup_rsv_type(ull key){
        static const std::map<ull, std::string> rsv_type_map = {
            {RS_ENT_ALU, "ALU"}, {RS_ENT_BRANCH, "BRANCH"},
            {RS_ENT_JAL, "JAL"}, {RS_ENT_JALR, "JALR"},
            {RS_ENT_MUL, "MUL"}, {RS_ENT_DIV, "DIV"},
            {RS_ENT_LDST, "LDST"}
        };
        auto it = rsv_type_map.find(key);
        return (it != rsv_type_map.end()) ? it->second : "UNKNOWN";
    }

    std::string lookup_src_a_sel(ull key){
        static const std::map<ull, std::string> src_a_sel_map = {
            {0, "RS1"}, {1, "PC"}, {2, "ZERO"}
        };
        auto it = src_a_sel_map.find(key);
        return (it != src_a_sel_map.end()) ? it->second : "UNKNOWN";
    }

    std::string lookup_src_b_sel(ull key){
        static const std::map<ull, std::string> src_b_sel_map = {
            {0, "RS2"}, {1, "IMM"}, {2, "FOUR"}, {3, "ZERO"}
        };
        auto it = src_b_sel_map.find(key);
        return (it != src_b_sel_map.end()) ? it->second : "UNKNOWN";
    }

    

    bool write_slot_if_running(REC_PIP_STAGE rps, pip_stat st, SlotWriterBase& writer){

        writer.add_slot_val(rps, pip_stat_to_string(st));
        return st == PS_RUNNING;
    }

    void write_issue(const std::string& stage_name,
                    SlotWriterBase& writer,
                    pip_stat st,
                    ull issue_idx,
                    bool is_oh_idx){

        writer.add_slot_val(RPS_ISSUE, "-----> "+ stage_name + "ISSUE Stage");
        if (!write_slot_if_running(RPS_ISSUE, st, writer)){
            return;
        }

        if (is_oh_idx){
            writer.add_slot_val(RPS_ISSUE, "issue idx: " + std::to_string(log2(issue_idx)));
        }else{
            writer.add_slot_val(RPS_ISSUE, "issue idx: " + std::to_string(issue_idx));

        }


    }

    std::string translate_instr_to_op_code(ull raw_instr){
        std::map<ull, std::string> dec_map = {
            {0b00'000'11, "LOAD"   }, {0b01'000'11, "STORE"  },
            {0b00'011'11, "MISCMEM"}, {0b11'000'11, "BRANCH" },
            {0b11'001'11, "JALR"   }, {0b11'011'11, "JAL"    },
            {0b00'100'11, "OP_IMM" }, {0b01'100'11, "OP"     },
            {0b00'101'11, "AUIPC"  }, {0b01'101'11, "LUI"    },
            {0b11'100'11, "SYSTEM" },
        };
        ull op_mask_bit = (1 << 7) - 1;
        ull op = raw_instr & op_mask_bit;
        return (dec_map.find(op) != dec_map.end()) ? dec_map[op] : "UNKNOWN";
    }


    std::string translate_alu_op_to_op(ull alu_op_idx){
        std::map<ull, std::string> alu_op_map = {
            { 0, "ADD"} , { 1, "SLL"} , { 4, "XOR"},
            { 6, "OR"}  , { 7, "AND"} , { 5, "SRL"},
            { 8, "SEQ"} , { 9, "SNE"} , {10, "SUB"},
            {11, "SRA"} , {12, "SLT"} , {13, "SGE"},
            {14, "SLTU"}, {15, "SGEU"}
        };
        return (alu_op_map.find(alu_op_idx) != alu_op_map.end()) ? alu_op_map[alu_op_idx] : "UNKNOWN";
    }

    /**
     * print
     */
    void SimState::Fetch::print_slot(SlotWriterBase& writer){

        if (!write_slot_if_running(RPS_FETCH, st, writer)){
            return;
        }
        writer.add_slot_val(RPS_FETCH, "pc : " + cvt_num2_hex_str(pc));

    }

    void SimState::Decode::print_slot(SlotWriterBase& writer){

        if (!write_slot_if_running(RPS_DECODE, st, writer)){
            return;
        }
        writer.add_slot_val(RPS_DECODE, "gennable "+ std::to_string(is_genable));
        writer.add_slot_val(RPS_DECODE, "PC" + cvt_num2_hex_str(pc));
        writer.add_slot_val(RPS_DECODE, "NPC" + cvt_num2_hex_str(npc));

        for (int i = 1; i <= 2; i++){
            writer.add_slot_val(RPS_DECODE, "- DEC " + std::to_string(i) + "-");
            ull sim_invalid = (i == 1) ? 0     : invalid2;
            ull sim_instr   = (i == 1) ? inst1 : inst2;

            if (sim_invalid){
                writer.add_slot_val(RPS_DECODE, "not_valid");
            }else{
                std::string dec_str = "valid OP: ";
                dec_str += translate_instr_to_op_code(sim_instr);
                writer.add_slot_val(RPS_DECODE, dec_str);
            }
        }
    }

    void SimState::DispInstr::print_slot(SlotWriterBase& writer){

        if (invalid){
                writer.add_slot_val(RPS_DISPATCH, "not_valid");
        }else{

            writer.add_slot_val(RPS_DISPATCH, "RS: " + lookup_rsv_type(rs_ent));
            writer.add_slot_val(RPS_DISPATCH, lookup_imm_type(imm_type));
            writer.add_slot_val(RPS_DISPATCH, "ALU: " + translate_alu_op_to_op(alu_op));

            writer.add_slot_val(RPS_DISPATCH, "is_br: " + std::to_string(is_branch) +
                                            "/is_sp: " + std::to_string(spec) +
                                            "/sp_tag: " + cvt_num2_bin_str(spec_tag));
            writer.add_slot_val(RPS_DISPATCH, "nextPc_" + cvt_num2_hex_str(pred_addr));

            std::string rd_usage = (rd_use != 0) ? "(USE)" : "(UNUSED)";
            writer.add_slot_val(RPS_DISPATCH, "RD: "+ rd_usage + " /ArchIdx: " +  std::to_string(rd_idx));

            std::string r1Usage = (rsUse_1 != 0) ? "(USE)" : "(UNUSED)";
            std::string sel_str = lookup_src_a_sel(rsSel_1);
            writer.add_slot_val(RPS_DISPATCH, "RS1: " + r1Usage + " /IDX:" + std::to_string(rsIdx_1) +
                                    " /SEL:" + sel_str);

            std::string r2Usage = (rsUse_2 != 0) ? "(USE)" : "(UNUSED)";
            std::string sel_str2 = lookup_src_b_sel(rsSel_2);
            writer.add_slot_val(RPS_DISPATCH, "RS2: " + r2Usage + " /IDX:" + std::to_string(rsIdx_2) +
                                    " /SEL:" + sel_str2);
        }

    }

    void SimState::Dispatch::print_slot(SlotWriterBase& writer){

        if (!write_slot_if_running(RPS_DISPATCH, st, writer)){
            return;
        }
        writer.add_slot_val(RPS_DISPATCH, "alu_rsv_able: " + std::to_string(is_alu_rsv_allocatable));
        writer.add_slot_val(RPS_DISPATCH, "br_rsv_able: " + std::to_string(is_branch_rsv_allocatable));
        writer.add_slot_val(RPS_DISPATCH, "is_renam: " + std::to_string(is_renamable));
        writer.add_slot_val(RPS_DISPATCH, "PC: " + cvt_num2_hex_str(pc));
        std::string internal_dep = str("S1EqDes: ") + (des_eq_src1? "1 " : "0 ") +
                                  str("S2EqDes: ") + (des_eq_src2? "1 " : "0 ");
        writer.add_slot_val(RPS_DISPATCH, internal_dep);

        writer.add_slot_val(RPS_DISPATCH, "---->instr0");
        dp1.print_slot(writer);
        writer.add_slot_val(RPS_DISPATCH, "---->instr1");
        dp2.print_slot(writer);
    }

    void SimState::RSV_BASE_ENTRY::print_slot(SlotWriterBase& writer, REC_PIP_STAGE rps){

        if (busy == 0){return;}

        std::string result0;
        result0 += std::to_string(idx) + "] ";

        result0 += "sb:" + std::string(sortbit ? "1" : "0") + " ";


        result0 += "pc:" + cvt_num2_hex_str(pc) + " ";
        result0 += "pd:" + std::to_string(rrftag) + " ";


        bool ready = (valid1 == 1) && (valid2 == 1);

        if (ready){
            result0 += "READY!";
        }else{
            result0 += "W:";
            if (!(valid1 == 1)){
                result0 += std::to_string(src1);
            }
            if (!(valid2 == 1)){
                result0 += "-";
                result0 += std::to_string(src2);
            }
        }
        writer.add_slot_val(rps, result0);

        ////// result 2 speculative meta data
        std::string result1;
        result1 += "spec: " + std::to_string(spec_bit) + " spt:" + cvt_num2_bin_str(spectag);

        writer.add_slot_val(rps, result1);


    }

    void SimState::RSV_BRANCH_ENTRY::print_slot(SlotWriterBase& writer, REC_PIP_STAGE rps){
        RSV_BASE_ENTRY::print_slot(writer, rps);
    }



    void SimState::RSV_MUL_ENTRY::print_slot(SlotWriterBase& writer, REC_PIP_STAGE rps){

        RSV_BASE_ENTRY::print_slot(writer, rps);
        if (busy == 0){return;}
        std::string r1s = (src1_signed) ? "s" : "u";
        std::string r2s = (src2_signed) ? "s" : "u";
        std::string hl  = (sel_lohi)    ? "h" : "l";
        writer.add_slot_val(rps,"mul_req: 1:" + r1s + " "
                         + "2:" + r2s + " "
                         + "sl:");
    }

    void SimState::RSV_BASE_ENTRY::print_detailed_slot(SlotWriterBase& writer,
                                                     REC_PIP_STAGE rps,
                                                     const COMMIT_STATE& commit_state){
        writer.add_slot_val(rps, "PC: " + cvt_num2_hex_str(pc));
        writer.add_slot_val(rps, "IMM: " + cvt_num2_hex_str(imm));
        writer.add_slot_val(rps, "ALU Op: " + translate_alu_op_to_op(alu_op) +
                               "/Spec: " + std::to_string(spec_bit) +
                               "/SpecTag: " + cvt_num2_bin_str(spectag));
        std::string sim_isRdUsed = dstval ? "(USE)" : "(UNUSED)";
        if (dstval){
            ull arch_idx = commit_state.com_entries[rrftag].rd_idx;
            writer.add_slot_val(rps, "RD phy: " + std::to_string(rrftag) + " arch: " + std::to_string(arch_idx));
        }else{
            writer.add_slot_val(rps, "RD phy: " + std::to_string(rrftag) + " arch(UNUSED)");
        }

        std::string sim_rs1Valid = valid1 ? "(valid)" : "(false)";
        std::string sim_rs1Sel   = lookup_src_a_sel(src1_sel);
        writer.add_slot_val(rps, "RS1" + sim_rs1Valid +
                               " /Data: " + std::to_string(src1) +
                               " /Sel: " + sim_rs1Sel);

        std::string sim_rs2Valid = valid2 ? "(valid)" : "(false)";
        std::string sim_rs2Sel   = lookup_src_b_sel(src2_sel);
        writer.add_slot_val(rps, "RS2" + sim_rs2Valid +
                               " /Data: " + std::to_string(src2) +
                               " /Sel: " + sim_rs2Sel);
    }

    void SimState::RSV_BRANCH_ENTRY::print_detailed_slot(SlotWriterBase& writer,
                                                     REC_PIP_STAGE rps,
                                                     const COMMIT_STATE& commit_state){
        ///// master call
        RSV_BASE_ENTRY::print_detailed_slot(writer, rps, commit_state);

        std::string result0 = "IMMBR: " + cvt_num2_hex_str(imm_br) + "/ PADDR: " + cvt_num2_hex_str(praddr);
        writer.add_slot_val(rps, result0);
        writer.add_slot_val(rps, "opcode: " + cvt_num2_hex_str(opcode));
    }

    void SimState::RSV_MUL_ENTRY::print_detailed_slot(SlotWriterBase& writer,
                                                     REC_PIP_STAGE rps,
                                                     const COMMIT_STATE& commit_state){
        ////// master call
        RSV_BASE_ENTRY::print_detailed_slot(writer, rps, commit_state);
        std::string result0 = "sign1: " + std::to_string(src1_signed) +
                              " /sign2: " + std::to_string(src2_signed) +
                              " /sel_hi: " + std::to_string(sel_lohi);
        writer.add_slot_val(rps, result0);
    }

    /**
     *  exec slot
     *
     */
    void SimState::EXEC_ALU_STATE::print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state){
        writer.add_slot_val(RPS_EXECUTE, "----> ALU Stage ");
        if (!write_slot_if_running(RPS_EXECUTE, st, writer)){return;}
        entry.print_detailed_slot(writer, RPS_EXECUTE, commit_state);
    }

    void SimState::EXEC_MUL_STATE::print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state){
        writer.add_slot_val(RPS_EXECUTE, "----> MUL Stage ");
        if (!write_slot_if_running(RPS_EXECUTE, st, writer)){return;}
        entry.print_detailed_slot(writer, RPS_EXECUTE, commit_state);
    }

    void SimState::EXEC_BRANCH_STATE::print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state, BC bc_state){
        writer.add_slot_val(RPS_EXECUTE, "----> BRANCH Stage ");
        if (!write_slot_if_running(RPS_EXECUTE, st, writer)){return;}

        if (bc_state.mis_pred){
            writer.add_slot_val(RPS_EXECUTE, "MISPRED");
        }
        if (bc_state.succ_pred){
            writer.add_slot_val(RPS_EXECUTE, "SUCCPRED");
        }

        entry.print_detailed_slot(writer, RPS_EXECUTE, commit_state);
    }

    void SimState::EXEC_LDST_STATE::print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state){
        writer.add_slot_val(RPS_EXECUTE, "----> LDST Stage ");
        if (write_slot_if_running(RPS_EXECUTE, st1, writer)){
            entry.print_detailed_slot(writer, RPS_EXECUTE, commit_state);
            writer.add_slot_val(RPS_EXECUTE, "eff_addr " + cvt_num2_hex_str(eff_addr));
        }


        writer.add_slot_val(RPS_EXECUTE, "-----> EXEC Stage 2");
        if (write_slot_if_running(RPS_EXECUTE, st2, writer)){
            writer.add_slot_val(RPS_EXECUTE, "rd_use:" + std::to_string(rd_use) +
                                           " /spec:" + std::to_string(spec) +
                                           " /spec_tag:" + cvt_num2_bin_str(spec_tag));
            writer.add_slot_val(RPS_EXECUTE, "stb_hit:" + std::to_string(st_buf_hit) +
                                           "stb_data:" + cvt_num2_hex_str(st_buf_data));
            writer.add_slot_val(RPS_EXECUTE, "load_data: " + std::to_string(load_data));
        }
    }

    void SimState::COMMIT_ENTRY::print_slot(SlotWriterBase& writer){

        //////// basic string for rob entry
        std::string entry_str = "E:" + std::to_string(idx);
        entry_str += "/fin:" + std::to_string(wb_fin);

        entry_str += "/rd:";
        if (rd_use){
            entry_str += std::to_string(rd_idx);
        }else{
            entry_str += "-";
        }

        if (store_bit){
            entry_str += "/Store";
        }
        writer.add_slot_val(RPS_COMMIT, entry_str);
    }


    void SimState::COMMIT_STATE::print_slot(SlotWriterBase& writer){
        writer.add_slot_val(RPS_COMMIT, "com_ptr:" + std::to_string(com_ptr));
        writer.add_slot_val(RPS_COMMIT, "com1Status:" + std::to_string(com1Status));
        writer.add_slot_val(RPS_COMMIT, "com2Status:" + std::to_string(com2Status));
        if (com1Status){
            com_entries[com_ptr % RRF_NUM].print_slot(writer);
        }
        if (com2Status){
            com_entries[(com_ptr + 1) % RRF_NUM].print_slot(writer);
        }

        if (is_prev_cycle_dp1){
            writer.add_slot_val(RPS_COMMIT, "-----------");
            writer.add_slot_val(RPS_COMMIT, "dispatched");
            com_entries[dp_pointer % RRF_NUM].print_slot(writer);
            if (is_prev_cycle_dp2){
                com_entries[(dp_pointer + 1) % RRF_NUM].print_slot(writer);
            }

        }

    }

    ////// store buffer stage

    void SimState::STORE_BUF_ENTRY::print_slot(SlotWriterBase& writer){

        if (!busy){return;}

        std::string result0;
        result0 += "[" + std::to_string(idx) + " ";
        result0 += "cpt:" + std::to_string(complete);
        result0 += "sp:" + std::to_string(spec) + " ";
        result0 += "sptg:" + cvt_num2_bin_str(spec_tag) + " ";
        writer.add_slot_val(RPS_STBUF, result0);

        std::string result1;
        result1 += "addr:" + cvt_num2_hex_str(mem_addr) + " ";
        result1 += "data:" + cvt_num2_hex_str(mem_data);
        writer.add_slot_val(RPS_STBUF, result1);
    }

    void SimState::STORE_BUF_STATE::print_slot(SlotWriterBase& writer){
        writer.add_slot_val(RPS_STBUF, "fin_ptr: " + std::to_string(fin_ptr));
        writer.add_slot_val(RPS_STBUF, "com_ptr: " + std::to_string(com_ptr));
        writer.add_slot_val(RPS_STBUF, "ret_ptr: " + std::to_string(ret_ptr));
        for (int idx = 0; idx < STBUF_ENT_NUM; idx++){
            entries[idx].print_slot(writer);
        }
        writer.add_slot_val(RPS_STBUF, "nb1 " + std::to_string(nb1));
        writer.add_slot_val(RPS_STBUF, "ne1 " + std::to_string(ne1));
        writer.add_slot_val(RPS_STBUF, "nb0 " + std::to_string(nb0));
        writer.add_slot_val(RPS_STBUF, "full_next " + std::to_string(full_next));
        writer.add_slot_val(RPS_STBUF, "empty_next " + std::to_string(empty_next));
    }

    /**
     * register
     ***/

    void SimState::MPFT_STATE::print_slot(SlotWriterBase& writer){
        for (int sp_idx = 0; sp_idx < SPECTAG_LEN; sp_idx++){
            std::string spec_tag_str = "v: " + std::to_string(valids[sp_idx]) + " sp: ";
            for (int idx = (SPECTAG_LEN - 1); idx >= 0; idx--){
                spec_tag_str += std::to_string(fix_table[sp_idx][idx]);
            }
            writer.add_slot_val(RPS_MPFT, spec_tag_str);
        }
    }

    void SimState::TAGGEN_STATE::print_slot(SlotWriterBase& writer){
        writer.add_slot_val(RPS_DECODE, "tag gen");
        writer.add_slot_val(RPS_DECODE, "branch depth" + std::to_string(brdepth));
        writer.add_slot_val(RPS_DECODE, "tag_reg " + cvt_num2_bin_str(tag_reg));
    }

    void SimState::ARF_STATE::print_slot(SlotWriterBase& writer, BC bc_prev){

        if (bc_prev.mis_pred){
            writer.add_slot_val(RPS_ARF, "CHANGE FROM MISPRED");
        }else if (bc_prev.succ_pred){
            writer.add_slot_val(RPS_ARF, "CHANGE FROM SUCCPRED");
        }

        for (int table_idx = 0; table_idx < (SPECTAG_LEN+1); table_idx++){
            writer.add_slot_val(RPS_ARF, "TABLEIDX " + std::to_string(table_idx));
            std::string re_name_val;
            for (int rrf_idx = 0; rrf_idx < REG_NUM; rrf_idx++){
                if (busy[table_idx][rrf_idx]){
                    re_name_val += std::to_string(rename[table_idx][rrf_idx]) + "|";
                }else{
                    re_name_val += "-|";
                }

                if ((rrf_idx % 8) == 7){
                    writer.add_slot_val(RPS_ARF, re_name_val);
                    re_name_val.clear();
                }
            }
        }

    }

    void SimState::RRF_STATE::print_slot(SlotWriterBase& writer){
        writer.add_slot_val(RPS_RRF, "free_num " + std::to_string(freenum));
        writer.add_slot_val(RPS_RRF, "req_ptr"   + std::to_string(req_ptr));
        writer.add_slot_val(RPS_RRF, "next_cycle" + std::to_string(next_rrf_cycle));
        // for (int rrf_idx = 0; rrf_idx < REG_NUM; rrf_idx++){
        //     writer.add_slot_val(RPS_RRF, "RRFIDX " + std::to_string(rrf_idx) +
        //                                 " /V: " + std::to_string(busy[rrf_idx]) +
        //                                 " /DATA:" + cvt_num2_hex_str(data[rrf_idx]));
        // }
    }

    void SimState::print_slot_window(SlotWriterBase& writer){
        fetch.print_slot(writer);
        decode.print_slot(writer);
        dispatch.print_slot(writer);

        /////// alu 1
        writer.add_slot_val(RPS_RSV, "---> ALU Stage 1");
        for (int i = 0; i < ALU_ENT_NUM; i++){
            rsv_alu1[i].print_slot(writer, RPS_RSV);
        }
        /////// alu 2
        writer.add_slot_val(RPS_RSV, "---> ALU Stage 2");
        for (int i = 0; i < ALU_ENT_NUM; i++){
            rsv_alu2[i].print_slot(writer, RPS_RSV);
        }
        writer.add_slot_val(RPS_RSV, "---> MUL Stage");
        for (int i = 0; i < MUL_ENT_NUM; i++){
            rsv_mul[i].print_slot(writer, RPS_RSV);
        }
        writer.add_slot_val(RPS_RSV, "---> BR Stage");
        for (int i = 0; i < BRANCH_ENT_NUM; i++){
            rsv_branch[i].print_slot(writer, RPS_RSV);
        }
        writer.add_slot_val(RPS_RSV, "---> LDST Stage");
        for (int i = 0; i < LDST_ENT_NUM; i++){
            rsv_ld_st[i].print_slot(writer, RPS_RSV);
        }

        write_issue("alu1", writer, st_issue_alu1  , idx_issue_alu1, true);
        write_issue("alu2", writer, st_issue_alu2  , idx_issue_alu2, true);
        write_issue("mul", writer, st_issue_mul   , idx_issue_mul  , true);
        write_issue("branch", writer, st_issue_branch, idx_issue_branch, false);
        write_issue("ldst", writer, st_issue_ldst  , idx_issue_ldst    , false);


        /////// execute unit

        exec_alu1.print_slot(writer, rob);
        exec_alu2.print_slot(writer, rob);
        exec_mul.print_slot(writer, rob);
        exec_branch.print_slot(writer, rob, bc_state);
        exec_ldst.print_slot(writer, rob);

        rob.print_slot(writer);
        stbuf.print_slot(writer);

        /////// register architecture

        mpft.print_slot  (writer);
        tag_gen.print_slot(writer);
        arf.print_slot   (writer, bc_prev);
        rrf.print_slot   (writer);
    }

}
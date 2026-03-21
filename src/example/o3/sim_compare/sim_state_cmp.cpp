//
// Created by tanawin on 22/12/25.
//

#include "sim_state.h"

namespace kathryn::o3{

    void print_state_mis_match(const std::string& stage_name, pip_stat lhs, pip_stat rhs){
        std::cout << TC_RED << stage_name << ": Mismatch on sim_state" << TC_DEF << std::endl;
        std::cout << TC_RED
                  << "lhs: " << pip_stat_to_string(lhs)
                  << " rhs: " << pip_stat_to_string(rhs)
                  << TC_DEF << std::endl;
    }

    bool check_and_print_sim_value_ull(ull lhs, ull rhs,
                               const std::string& stage_name,
                               const std::string& sim_name){
        if (lhs != rhs){
            std::cout << TC_RED << stage_name << " with sim_type "<< sim_name
                      << ": LHS: " << lhs << " RHS: " << rhs << TC_DEF << std::endl;
            return false;
        }
        return true;
    }

    bool check_and_print_sim_value_bool(bool lhs, bool rhs,
                               const std::string& stage_name,
                               const std::string& sim_name){
        if (lhs != rhs){
            std::cout << TC_RED << stage_name << " with sim_type "<< sim_name
                      << ": LHS: " << lhs << " RHS: " << rhs << TC_DEF << std::endl;
            return false;
        }
        return true;
    }




    bool SimState::is_state_should_check_further(pip_stat ps){ return ps == PS_RUNNING;}

    /**
     * fetch check
     ***/

    bool SimState::Fetch::compare(const Fetch& rhs) const{

        if (st != rhs.st){
            print_state_mis_match("fetch stage", st, rhs.st);
            return false;
        }

        if (st != PS_RUNNING){ return true;}

        bool compare_result = true;
        compare_result &= check_and_print_sim_value_ull(pc, rhs.pc, "fetch", "pc");
        return compare_result;
    }

    /**
     *
     * decode
     *
     */
    bool SimState::Decode::compare(const Decode& rhs) const{

        if (st != rhs.st){
            print_state_mis_match("decode stage", st, rhs.st);
            return false;
        }

        if (st != PS_RUNNING){return true;}

        bool compare_result = true;
        compare_result &= check_and_print_sim_value_ull(inst1 , rhs.inst1 , "decode", "inst1" );
        compare_result &= check_and_print_sim_value_ull(invalid2 , rhs.invalid2 , "decode", "invalid2" );

        if (invalid2 != 0){
            compare_result &= check_and_print_sim_value_ull(inst2 , rhs.inst2 , "decode", "inst2" );
        }

        compare_result &= check_and_print_sim_value_ull(pc    , rhs.pc    , "decode", "pc" );
        compare_result &= check_and_print_sim_value_ull(npc   , rhs.npc   , "decode", "npc");

        return compare_result;

    }

    /**
     *
     *  dispatch
     */
    bool SimState::DispInstr::compare(const DispInstr& rhs) const{

        if (invalid != rhs.invalid){
            check_and_print_sim_value_ull(invalid  , rhs.invalid , "dispatch", "invalid  ");
            return false;
        }
        if (invalid){
            return true;
        }
        bool compare_result = true;
        compare_result &= check_and_print_sim_value_ull(imm_type  , rhs.imm_type , "dispatch", "imm_type  ");
        compare_result &= check_and_print_sim_value_ull(alu_op     , rhs.alu_op    , "dispatch", "alu_op     ");
        compare_result &= check_and_print_sim_value_ull(rs_ent     , rhs.rs_ent    , "dispatch", "rs_ent     ");
        compare_result &= check_and_print_sim_value_ull(is_branch  , rhs.is_branch , "dispatch", "is_branch  ");
        compare_result &= check_and_print_sim_value_ull(pred_addr , rhs.pred_addr, "dispatch", "pred_addr ");
        compare_result &= check_and_print_sim_value_ull(spec      , rhs.spec     , "dispatch", "spec      ");
        compare_result &= check_and_print_sim_value_ull(spec_tag   , rhs.spec_tag  , "dispatch", "spec_tag   ");
        compare_result &= check_and_print_sim_value_ull(rd_idx     , rhs.rd_idx    , "dispatch", "rd_idx     ");
        compare_result &= check_and_print_sim_value_ull(rd_use     , rhs.rd_use    , "dispatch", "rd_use     ");
        compare_result &= check_and_print_sim_value_ull(rsIdx_1   , rhs.rsIdx_1  , "dispatch", "rsIdx_1   ");
        compare_result &= check_and_print_sim_value_ull(rsSel_1   , rhs.rsSel_1  , "dispatch", "rsSel_1   ");
        compare_result &= check_and_print_sim_value_ull(rsUse_1   , rhs.rsUse_1  , "dispatch", "rsUse_1   ");
        compare_result &= check_and_print_sim_value_ull(rsIdx_2   , rhs.rsIdx_2  , "dispatch", "rsIdx_2   ");
        compare_result &= check_and_print_sim_value_ull(rsSel_2   , rhs.rsSel_2  , "dispatch", "rsSel_2   ");
        compare_result &= check_and_print_sim_value_ull(rsUse_2   , rhs.rsUse_2  , "dispatch", "rsUse_2   ");

        return compare_result;
    }

    bool SimState::Dispatch::compare(const Dispatch& rhs) const{

        if (st != rhs.st){
            print_state_mis_match("dispatch stage", st, rhs.st);
            return false;
        }

        if (st != PS_RUNNING){return true;}

        bool compare_result = true;
        compare_result &= check_and_print_sim_value_ull(pc, rhs.pc, "dispatch", "pc");
        compare_result &= check_and_print_sim_value_ull(des_eq_src1, rhs.des_eq_src1, "dispatch", "des_eq_src1");
        compare_result &= check_and_print_sim_value_ull(des_eq_src2, rhs.des_eq_src2, "dispatch", "des_eq_src2");
        compare_result &= dp1.compare(rhs.dp1);
        compare_result &= dp2.compare(rhs.dp2);

        return compare_result;
    }

    //////// RSV_BASE_ENTRY


    bool SimState::RSV_BASE_ENTRY::compare(const RSV_BASE_ENTRY& rhs) const{

        if (!check_and_print_sim_value_ull(busy, rhs.busy, "RSV_BASE_ENTRY", "busy")){
            return false;
        }
        bool compare_result = true;
        if (busy == 0){ return compare_result; }



        compare_result &= check_and_print_sim_value_ull(sortbit, rhs.sortbit, name, "sortbit");
        compare_result &= check_and_print_sim_value_ull(pc     , rhs.pc     , name, "pc"     );
        compare_result &= check_and_print_sim_value_ull(rrftag , rhs.rrftag , name, "rrftag" );
        compare_result &= check_and_print_sim_value_ull(dstval , rhs.dstval , name, "dstval" );
        compare_result &= check_and_print_sim_value_ull(alu_op , rhs.alu_op , name, "alu_op" );
        compare_result &= check_and_print_sim_value_ull(spectag, rhs.spectag, name, "spectag");

        if (valid1 && src1_sel == SRC_A_RS1){
            compare_result &= check_and_print_sim_value_ull(src1, rhs.src1, name, "src1");
        }
        if (valid2 && src2_sel == SRC_B_RS2){
            if (src2_sel == SRC_B_RS2){
                compare_result &= check_and_print_sim_value_ull(src2, rhs.src2, name, "src2");
            }else if (src2_sel == SRC_B_IMM){
                compare_result &= check_and_print_sim_value_ull(imm, rhs.imm, name, "imm");
            }

        }
        return compare_result;
    }

    //////// RSV_BRANCH_ENTRY

    bool SimState::RSV_BRANCH_ENTRY::compare(const RSV_BRANCH_ENTRY& rhs) const{

        bool compare_result = RSV_BASE_ENTRY::compare(rhs);

        if (busy == 0){ return compare_result; }

        compare_result &= check_and_print_sim_value_ull(imm_br, rhs.imm_br, name, "immbr");
        compare_result &= check_and_print_sim_value_ull(praddr, rhs.praddr, name, "praddr");
        compare_result &= check_and_print_sim_value_ull(opcode, rhs.opcode, name, "opcode");

        return compare_result;

    }

    bool SimState::RSV_MUL_ENTRY::compare(const RSV_MUL_ENTRY& rhs) const{

        bool compare_result = RSV_BASE_ENTRY::compare(rhs);

        if (busy == 0){ return compare_result; }

        compare_result &= check_and_print_sim_value_ull(src1_signed, rhs.src1_signed, name, "src1_signed");
        compare_result &= check_and_print_sim_value_ull(src2_signed, rhs.src2_signed, name, "src2_signed");
        compare_result &= check_and_print_sim_value_ull(sel_lohi   , rhs.sel_lohi   , name, "sel_lohi");

        return compare_result;

    }

    /**
     * exec unit
     *
     ***/

    bool SimState::EXEC_ALU_STATE::compare(const EXEC_ALU_STATE& rhs) const{
        if (st != rhs.st){
            print_state_mis_match("exec ALU stage", st, rhs.st);
            return false;
        }
        return entry.compare(rhs.entry);
    }

    bool SimState::EXEC_MUL_STATE::compare(const EXEC_MUL_STATE& rhs) const{
        if (st != rhs.st){
            print_state_mis_match("exec mul stage", st, rhs.st);
            return false;
        }
        return entry.compare(rhs.entry);
    }

    bool SimState::EXEC_BRANCH_STATE::compare(const EXEC_BRANCH_STATE& rhs) const{
        if (st != rhs.st){
            print_state_mis_match("exec branch stage", st, rhs.st);
            return false;
        }
        return entry.compare(rhs.entry);
    }

    bool SimState::EXEC_LDST_STATE::compare(const EXEC_LDST_STATE& rhs) const{
        bool compare_result = true;
        if (st1 == rhs.st1){
            compare_result &= entry.compare(rhs.entry);
            if (st1 == PS_RUNNING){
                compare_result &= check_and_print_sim_value_ull(eff_addr, rhs.eff_addr, "exec LDST", "eff_addr");
            }
        }else{
            print_state_mis_match("exec LDST stage", st1, rhs.st1);
            compare_result = false;
        }

        if (st2 == rhs.st2){
            if (st2 == PS_RUNNING){
                compare_result &= check_and_print_sim_value_ull(rrftag   , rhs.rrftag   , "exec LDST", "rrftag"   );
                compare_result &= check_and_print_sim_value_ull(rd_use    , rhs.rd_use    , "exec LDST", "rd_use"    );
                compare_result &= check_and_print_sim_value_ull(spec     , rhs.spec     , "exec LDST", "spec"     );
                compare_result &= check_and_print_sim_value_ull(spec_tag  , rhs.spec_tag  , "exec LDST", "spec_tag"  );
                if (rd_use != 0){
                    compare_result &= check_and_print_sim_value_ull(st_buf_hit , rhs.st_buf_hit , "exec LDST", "st_buf_hit" );
                    if (st_buf_hit != 0){
                        compare_result &= check_and_print_sim_value_ull(st_buf_data, rhs.st_buf_data, "exec LDST", "st_buf_data");
                    }else{
                        compare_result &= check_and_print_sim_value_ull(load_data, rhs.load_data, "exec LDST", "load data");
                    }
                }
            }

        }else{
            print_state_mis_match("exec1 LDST stage", st1, rhs.st1);
            compare_result = false;

        }




        return compare_result;
    }


    /**
     *
     *  commit stage
     */
    bool SimState::COMMIT_ENTRY::compare(const COMMIT_ENTRY& rhs) const{

        bool compare_result = true;

        compare_result &= check_and_print_sim_value_ull(wb_fin   , rhs.wb_fin   , "COMMIT entry " + std::to_string(idx), "wb_fin");
        if (wb_fin == 0){
            return compare_result;
        }
        compare_result &= check_and_print_sim_value_ull(store_bit, rhs.store_bit, "COMMIT entry " + std::to_string(idx), "store_bit");
        compare_result &= check_and_print_sim_value_ull(rd_use   , rhs.rd_use   , "COMMIT entry " + std::to_string(idx), "rd_use");
        compare_result &= check_and_print_sim_value_ull(rd_idx   , rhs.rd_idx   , "COMMIT entry " + std::to_string(idx), "rd_idx");

        return compare_result;

    }

    bool SimState::COMMIT_STATE::compare(const COMMIT_STATE& rhs, ull req_ptr) const{
        bool compare_result = true;
        compare_result &= check_and_print_sim_value_ull(com_ptr    , rhs.com_ptr    , "COMMIT", "com_ptr"    );
        compare_result &= check_and_print_sim_value_bool(com1Status, rhs.com1Status, "COMMIT", "com1Status");
        compare_result &= check_and_print_sim_value_bool(com2Status, rhs.com2Status, "COMMIT", "com2Status");
        for (ull rrf_idx = com_ptr; rrf_idx != req_ptr; rrf_idx = (rrf_idx+1)%RRF_NUM){
            compare_result &= com_entries[rrf_idx].compare(rhs.com_entries[rrf_idx]);
        }

        return compare_result;
    }

    /**
     * store buffer state
     */

    bool SimState::STORE_BUF_ENTRY::compare(const STORE_BUF_ENTRY& rhs) const{

        if (!check_and_print_sim_value_bool(busy    , rhs.busy,     "STORE_BUF", "busy")){
            return false;
        }

        if (busy == 0){ return true; }

        return check_and_print_sim_value_ull (complete, rhs.complete, "STORE_BUF", "complete") &&
               check_and_print_sim_value_ull (spec    , rhs.spec,     "STORE_BUF", "spec") &&
               check_and_print_sim_value_ull (spec_tag , rhs.spec_tag,  "STORE_BUF", "spec_tag") &&
               check_and_print_sim_value_ull (mem_addr, rhs.mem_addr, "STORE_BUF", "mem_addr") &&
               check_and_print_sim_value_ull (mem_data, rhs.mem_data, "STORE_BUF", "mem_data");
    }

    bool SimState::STORE_BUF_STATE::compare(const STORE_BUF_STATE& rhs) const{
        bool compare_result = true;
        compare_result &= check_and_print_sim_value_ull(fin_ptr, rhs.fin_ptr, "STORE_BUF", "fin_ptr");
        compare_result &= check_and_print_sim_value_ull(com_ptr, rhs.com_ptr, "STORE_BUF", "com_ptr");
        compare_result &= check_and_print_sim_value_ull(ret_ptr, rhs.ret_ptr, "STORE_BUF", "ret_ptr");
        for (int idx = 0; idx < STBUF_ENT_NUM; idx++){
            compare_result &= entries[idx].compare(rhs.entries[idx]);
        }
        return compare_result;
    }

     /**
     *
     * REG architecture
     *
     */
    bool  SimState::MPFT_STATE::compare(const MPFT_STATE& rhs) const{

        bool compare_result = true;
        for (int idx = 0; idx < SPECTAG_LEN; idx++){
                compare_result &= check_and_print_sim_value_bool(valids[idx], rhs.valids[idx], "MPFT", "valids" + std::to_string(idx));
            if (valids[idx]){
                for (int fix_idx = 0; fix_idx < SPECTAG_LEN; fix_idx++){
                    compare_result &= check_and_print_sim_value_bool(fix_table[idx][fix_idx], rhs.fix_table[idx][fix_idx],
                                                               "MPFT", "fix_table idx: " + std::to_string(idx) +
                                                               " fix_idx: " + std::to_string(fix_idx));
                }
            }

        }
        return compare_result;
    }

    bool SimState::TAGGEN_STATE::compare(const TAGGEN_STATE& rhs) const{
        return check_and_print_sim_value_ull(brdepth, rhs.brdepth, "TAGGEN", "brdepth") &&
               check_and_print_sim_value_ull(tag_reg , rhs.tag_reg , "TAGGEN", "tag_reg" );
    }


    bool SimState::ARF_STATE::compare(const ARF_STATE& rhs) const{

        bool compare_result = true;
        for (int table_idx = 0; table_idx <= SPECTAG_LEN; table_idx++){

            for (int arch_idx = 0; arch_idx < REG_NUM; arch_idx++){
                compare_result &= check_and_print_sim_value_bool(busy[table_idx][arch_idx],
                                                          rhs.busy[table_idx][arch_idx],
                                                          "ARF",
                                                          "busy spec: " + std::to_string(table_idx) +
                                                          " arch_idx: " + std::to_string(arch_idx));
                if (busy[table_idx][arch_idx]){
                    compare_result &= check_and_print_sim_value_ull(rename[table_idx][arch_idx],
                                                              rhs.rename[table_idx][arch_idx], "ARF",
                                                              "arch_rem spec: " + std::to_string(table_idx) +
                                                              "arch_idx: " + std::to_string(arch_idx)
                    );
                }
            }
        }
        return compare_result;
    }

    bool SimState::RRF_STATE::compare(const RRF_STATE& rhs, ull com_ptr) const{
        bool compare_result = true;
        for (ull idx = com_ptr; idx != req_ptr; idx = (idx+1)%RRF_NUM){ ///// turn around check

            compare_result &= check_and_print_sim_value_bool(busy[idx], rhs.busy[idx], "RRF", "valids" + std::to_string(idx));
            if (busy[idx]){
                compare_result &= check_and_print_sim_value_ull(data[idx], rhs.data[idx],
                                                           "RRF", "phy_idx: " + std::to_string(idx));
            }
        }

        compare_result &= check_and_print_sim_value_ull(freenum     , rhs.freenum     , "RRF", "freenum");
        compare_result &= check_and_print_sim_value_ull(req_ptr      , rhs.req_ptr      , "RRF", "req_ptr");
        compare_result &= check_and_print_sim_value_ull(next_rrf_cycle, rhs.next_rrf_cycle, "RRF", "next_rrf_cycle");

        return compare_result;
    }

    bool SimState::compare(SimState& rhs) const{

        bool compare_result = true;

        compare_result &= fetch.compare(rhs.fetch);
        compare_result &= decode.compare(rhs.decode);
        compare_result &= dispatch.compare(rhs.dispatch);

        ////// rsv compare
        for (int rsv_idx = 0; rsv_idx < ALU_ENT_NUM; rsv_idx++){
            compare_result &= rsv_alu1[rsv_idx].compare(rhs.rsv_alu1[rsv_idx]);
        }
        for (int rsv_idx = 0; rsv_idx < ALU_ENT_NUM; rsv_idx++){
            compare_result &= rsv_alu2[rsv_idx].compare(rhs.rsv_alu2[rsv_idx]);
        }
        for (int rsv_idx = 0; rsv_idx < MUL_ENT_NUM; rsv_idx++){
            compare_result &= rsv_mul[rsv_idx].compare(rhs.rsv_mul[rsv_idx]);
        }
        for (int rsv_idx = 0; rsv_idx < BRANCH_ENT_NUM; rsv_idx++){
            compare_result &= rsv_branch[rsv_idx].compare(rhs.rsv_branch[rsv_idx]);
        }
        for (int rsv_idx = 0; rsv_idx < LDST_ENT_NUM; rsv_idx++){
            compare_result &= rsv_ld_st[rsv_idx].compare(rhs.rsv_ld_st[rsv_idx]);
        }

        compare_result &= exec_alu1.compare(rhs.exec_alu1);
        compare_result &= exec_alu2.compare(rhs.exec_alu2);
        compare_result &= exec_mul.compare(rhs.exec_mul);
        compare_result &= exec_branch.compare(rhs.exec_branch);
        compare_result &= exec_ldst.compare(rhs.exec_ldst);

        compare_result &= rob.compare(rhs.rob, rrf.req_ptr);
        compare_result &= stbuf.compare(rhs.stbuf);

        //// register architecture
        compare_result &= mpft.compare(rhs.mpft);
        compare_result &= tag_gen.compare(rhs.tag_gen);
        compare_result &= arf.compare(rhs.arf);
        compare_result &= rrf.compare(rhs.rrf, rob.com_ptr);

        return compare_result;
    }

}
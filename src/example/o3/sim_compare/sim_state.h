//
// Created by tanawin on 22/12/25.
//

#ifndef SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H
#define SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H

#include "util/file_writer/slot_writer/slot_writer_base.h"
#include "kathryn.h"
#include "sim_state.h"
#include "example/o3/core/mpft.h"
#include "example/o3/core/parameter.h"
#include "example/o3/sim_share/rec_pip_stage.h"


namespace kathryn::o3{

    enum pip_stat {PS_IDLE, PS_STALL, PS_RUNNING};

    inline const char* pip_stat_to_string(pip_stat st) {
        switch (st) {
        case PS_IDLE   : return "IDLE";
        case PS_STALL  : return "STALL";
        case PS_RUNNING: return "RUNNING";
        default:         return "UNKNOWN";
        }
    }


    struct SimState{
        virtual ~SimState() = default;

        struct BC{
            bool mis_pred  = false;
            bool succ_pred = false;
        } bc_state, bc_prev;



        struct Fetch{
            pip_stat st = PS_IDLE;
            ull     pc = 0;
            bool compare(const Fetch& rhs) const;
            void print_slot(SlotWriterBase& writer);


        } fetch;

        struct Decode{
            pip_stat st       = PS_IDLE;
            ull     inst1    = 0;
            ull     invalid2 = 0;
            ull     inst2    = 0;
            ull     pc       = 0;
            ull     npc      = 0;
            /** not compare*/
            bool is_genable = false;

            bool compare(const Decode& rhs) const;
            void print_slot(SlotWriterBase& writer);

        } decode;

        struct DispInstr{
            //// TODO check predict cond mul use case
            int idx       = 0;
            ull invalid   = 0;
            ull imm_type  = 0;
            ull alu_op     = 0;
            ull rs_ent     = 0;
            ull is_branch  = 0;
            ull pred_addr = 0;
            ull spec      = 0;
            ull spec_tag   = 0;
            ull rd_idx     = 0;
            ull rd_use     = 0;
            ull rsIdx_1   = 0;
            ull rsSel_1   = 0;
            ull rsUse_1   = 0;
            ull rsIdx_2   = 0;
            ull rsSel_2   = 0;
            ull rsUse_2   = 0;

            explicit DispInstr(int idx): idx(idx){}

            bool compare(const DispInstr& rhs) const;
            void print_slot(SlotWriterBase& writer);
        };

        struct Dispatch{
            pip_stat st = PS_IDLE;
            ///// shared signal
            ull pc        = 0;
            ull des_eq_src1 = 0;
            ull des_eq_src2 = 0;
            ///// individual instruction
            DispInstr dp1 {0};
            DispInstr dp2 {1};
            /** not compare*/
            bool is_alu_rsv_allocatable = false;
            bool is_branch_rsv_allocatable = false;
            bool is_renamable = false;

            bool compare(const Dispatch& rhs) const;
            void print_slot(SlotWriterBase& writer);

        } dispatch;

        struct COMMIT_STATE;
        struct RSV_BASE_ENTRY{
            std::string name;
            int idx     = 0;
            ull busy    = 0;
            ull sortbit = 0; //// incase inorder rsv this bit is discarded
            ull pc      = 0; //// mul and ldst discard it
            ull imm     = 0;
            ull rrftag  = 0;
            ull dstval  = 0;
            ull alu_op  = 0; //////  mul and ldst discard it
            ull spec_bit = 0;
            ull spectag = 0;

            ull src1     = 0, src2     = 0;
            ull src1_sel = 0, src2_sel = 0; ///// ldst discards it
            ull valid1   = 0, valid2   = 0;

            bool compare(const RSV_BASE_ENTRY& rhs) const;
            void print_slot(SlotWriterBase& writer, REC_PIP_STAGE rps);
            void print_detailed_slot(SlotWriterBase& writer,
                                   REC_PIP_STAGE rps,
                                   const COMMIT_STATE& commit_state);
        };


        struct RSV_BRANCH_ENTRY: RSV_BASE_ENTRY{
            ull imm_br = 0;
            ull praddr = 0;
            ull opcode = 0;

            bool compare(const RSV_BRANCH_ENTRY& rhs) const;
            void print_slot(SlotWriterBase& writer, REC_PIP_STAGE rps);
            void print_detailed_slot(SlotWriterBase& writer,
                                   REC_PIP_STAGE rps,
                                   const COMMIT_STATE& commit_state);
        };

        struct RSV_MUL_ENTRY: RSV_BASE_ENTRY{
            ull src1_signed = 0;
            ull src2_signed = 0;
            ull sel_lohi    = 0;

            bool compare(const RSV_MUL_ENTRY& rhs) const;
            void print_slot(SlotWriterBase& writer, REC_PIP_STAGE rps);
            void print_detailed_slot(SlotWriterBase& writer,
                                   REC_PIP_STAGE rps,
                                   const COMMIT_STATE& commit_state);
        };

        RSV_BASE_ENTRY rsv_alu1[ALU_ENT_NUM] {};
        RSV_BASE_ENTRY rsv_alu2[ALU_ENT_NUM] {};
        RSV_MUL_ENTRY  rsv_mul[MUL_ENT_NUM]   {};

        RSV_BRANCH_ENTRY rsv_branch[BRANCH_ENT_NUM]{};
        RSV_BASE_ENTRY   rsv_ld_st[LDST_ENT_NUM]    {};

        ////////// do not compare
        pip_stat st_issue_alu1   = PS_IDLE; ull idx_issue_alu1   = 0;
        pip_stat st_issue_alu2   = PS_IDLE; ull idx_issue_alu2   = 0;
        pip_stat st_issue_mul    = PS_IDLE; ull idx_issue_mul    = 0;
        pip_stat st_issue_branch = PS_IDLE; ull idx_issue_branch = 0;
        pip_stat st_issue_ldst   = PS_IDLE; ull idx_issue_ldst   = 0;

        /**
         * execute unit
         */
        struct EXEC_ALU_STATE{
            pip_stat st = PS_IDLE;
            RSV_BASE_ENTRY entry{};

            bool compare(const EXEC_ALU_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state);
        } exec_alu1, exec_alu2;

        struct EXEC_MUL_STATE{
            pip_stat st = PS_IDLE;
            RSV_MUL_ENTRY entry {};

            bool compare(const EXEC_MUL_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state);
        } exec_mul;

        struct EXEC_BRANCH_STATE{
            pip_stat st = PS_IDLE;
            RSV_BRANCH_ENTRY entry{};

            //// not compare

            bool compare(const EXEC_BRANCH_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state, BC bc);
        } exec_branch;

        struct EXEC_LDST_STATE{
            pip_stat st1 = PS_IDLE;
            pip_stat st2 = PS_IDLE;
            RSV_BASE_ENTRY entry{};
            ull     eff_addr = 0;


            ull rrftag    = 0;
            ull rd_use     = 0;
            ull spec      = 0;
            ull spec_tag   = 0;
            ull st_buf_data = 0;
            ull st_buf_hit  = 0;
            ull load_data  = 0;

            bool compare(const EXEC_LDST_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer, COMMIT_STATE& commit_state);
        }exec_ldst;

        ////// commit stage

        struct COMMIT_ENTRY{
            int idx       = 0;
            ull wb_fin     = 0;
            ull store_bit  = 0;
            ull rd_use     = 0;
            ull rd_idx     = 0;

            bool compare(const COMMIT_ENTRY& rhs) const;
            void print_slot(SlotWriterBase& writer);
        };

        struct COMMIT_STATE{
            ull  com_ptr     = 0;
            bool com1Status = false;
            bool com2Status = false;
            COMMIT_ENTRY com_entries[RRF_NUM]{};
            //////// do not compare this section
            bool is_prev_cycle_dp1 = false;
            bool is_prev_cycle_dp2 = false;
            ull  dp_pointer = 0;

            COMMIT_STATE(){
                for (int i = 0; i < RRF_NUM; i++){
                    com_entries[i].idx = i;
                }
            }

            bool compare(const COMMIT_STATE& rhs, ull req_ptr) const;
            void print_slot(SlotWriterBase& writer);
        } rob;

        ////// store buffer stage

        struct STORE_BUF_ENTRY{
            int  idx        = 0;
            bool busy       = false;
            ull  complete   = 0;
            ull  spec       = 0;
            ull  spec_tag    = 0;
            ull  mem_addr   = 0;
            ull  mem_data   = 0;

            bool compare(const STORE_BUF_ENTRY& rhs) const;
            void print_slot(SlotWriterBase& writer);
        };

        struct STORE_BUF_STATE{
            ull fin_ptr = 0;
            ull com_ptr = 0;
            ull ret_ptr = 0;
            STORE_BUF_ENTRY entries[STBUF_ENT_NUM]{};
            ///// not compare
            ull  nb1    = 0;
            ull  ne1    = 0;
            ull  nb0    = 0;
            bool full_next = false;
            bool empty_next = false;

            STORE_BUF_STATE(){
                for (int i = 0; i < STBUF_ENT_NUM; i++){
                    entries[i].idx = i;
                }
            }


            bool compare(const STORE_BUF_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer);
        } stbuf;

        /**
         * register architecture
         ***/

        struct MPFT_STATE{
            bool valids  [SPECTAG_LEN]{};
            bool fix_table[SPECTAG_LEN][SPECTAG_LEN]{};

            bool compare(const MPFT_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer);
        } mpft;

        struct TAGGEN_STATE{
            ull brdepth = 0;
            ull tag_reg  = 0;

            bool compare(const TAGGEN_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer);
        }tag_gen;

        struct ARF_STATE{
            //// the last one is master
            bool busy  [SPECTAG_LEN+1][REG_NUM]{};
            ull  rename[SPECTAG_LEN+1][REG_NUM]{};

            bool compare(const ARF_STATE& rhs) const;
            void print_slot(SlotWriterBase& writer, BC bc_prev);

        }arf;

        struct RRF_STATE{
            bool busy[RRF_NUM]{};
            ull  data[RRF_NUM]{};

            ull freenum      = 0;
            ull req_ptr       = 0;
            ull next_rrf_cycle = 0;

            /**
             * check only on com_ptr to req_ptr)
             */
            bool compare(const RRF_STATE& rhs, ull com_ptr) const;
            void print_slot(SlotWriterBase& writer);
        }rrf;


        SimState(){

            /////// set name for all rsv
            for (int i = 0; i < ALU_ENT_NUM; i++){
                rsv_alu1[i].name = "RSV_ALU1_" + std::to_string(i);
                rsv_alu1[i].idx  = i;
                rsv_alu2[i].name = "RSV_ALU2_" + std::to_string(i);
                rsv_alu2[i].idx  = i;
            }
            for (int i = 0; i < MUL_ENT_NUM; i++){
                rsv_mul[i].name = "RSV_MUL_" + std::to_string(i);
                rsv_mul[i].idx  = i;
            }
            for (int i = 0; i < BRANCH_ENT_NUM; i++){
                rsv_branch[i].name = "RSV_BRANCH_" + std::to_string(i);
                rsv_branch[i].idx  = i;
            }
            for (int i = 0; i < LDST_ENT_NUM; i++){
                rsv_ld_st[i].name = "RSV_LDST_" + std::to_string(i);
                rsv_ld_st[i].idx  = i;
            }
        }

        virtual void recruit_value()     = 0;
        virtual void recruit_next_cycle() = 0;

        bool compare(SimState& rhs) const;
        virtual void print_slot_window(SlotWriterBase& writer);

        bool is_state_should_check_further(pip_stat ps);
        
    };
    
}

#endif //SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H
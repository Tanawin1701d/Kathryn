//
// Created by tanawin on 22/12/25.
//

#ifndef SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H
#define SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H

#include "util/fileWriter/slotWriter/wslotWriter.h"
#include "kathryn.h"
#include "example/o3/core/mpft.h"
#include "example/o3/core/parameter.h"


namespace kathryn::o3{

    enum pipStat {IDLE, STALL, KILLED, RUNNING};

    inline const char* pipStatToString(pipStat st) {
        switch (st) {
        case IDLE:    return "IDLE";
        case STALL:   return "STALL";
        case KILLED:  return "KILLED";
        case RUNNING: return "RUNNING";
        default:      return "UNKNOWN";
        }
    }


    struct SimState{

        struct Fetch{
            pipStat st = IDLE;
            ull     pc = 0;
            [[nodiscard]] bool compare(const Fetch& rhs) const;


        } fetch;

        struct Decode{
            pipStat st = IDLE;
            ull inst1    = 0;
            ull invalid2 = 0;
            ull inst2    = 0;
            ull pc       = 0;
            ull npc      = 0;
            ull bhr      = 0;

            [[nodiscard]] bool compare(const Decode& rhs) const;
        } decode;

        struct DispInstr{
            int idx       = 0;
            ull invalid   = 0;
            ull imm_type  = 0;
            ull aluOp     = 0;
            ull rsEnt     = 0;
            ull isBranch  = 0;
            ull pred_addr = 0;
            ull spec      = 0;
            ull specTag   = 0;
            ull rdIdx     = 0;
            ull rdUse     = 0;
            ull rsIdx_1   = 0;
            ull rsSel_1   = 0;
            ull rsUse_1   = 0;
            ull rsIdx_2   = 0;
            ull rsSel_2   = 0;
            ull rsUse_2   = 0;

            explicit DispInstr(int idx): idx(idx){}

            bool compare(const DispInstr& rhs) const;
        };

        struct Dispatch{
            pipStat st = IDLE;
            ///// shared signal
            ull pc        = 0;
            ull desEqSrc1 = 0;
            ull desEqSrc2 = 0;
            ///// individual instruction
            DispInstr dp0 {0};
            DispInstr dp1 {1};
            bool compare(const Dispatch& rhs) const;

        } dispatchShared;

        struct RSV_BASE_ENTRY{
            std::string name;
            ull busy    = 0;
            ull sortbit = 0; //// incase inorder rsv this bit is discarded
            ull pc      = 0; //// mul and ldst discard it
            ull imm     = 0;
            ull rrftag  = 0;
            ull dstval  = 0;
            ull alu_op  = 0; //////  mul and ldst discard it
            ull spectag = 0;

            ull src1     = 0, src2     = 0;
            ull src1_sel = 0, src2_sel = 0; ///// ldst discards it
            ull valid1   = 0, valid2   = 0;

            bool compare(const RSV_BASE_ENTRY& rhs) const;
        };


        struct RSV_BRANCH_ENTRY: RSV_BASE_ENTRY{
            ull bhr    = 0;
            ull prcond = 0;
            ull praddr = 0;
            ull opcode = 0;

            bool compare(const RSV_BRANCH_ENTRY& rhs) const;
        };

        struct RSV_MUL_ENTRY: RSV_BASE_ENTRY{
            ull src1_signed = 0;
            ull src2_signed = 0;
            ull sel_lohi    = 0;

            bool compare(const RSV_MUL_ENTRY& rhs) const;
        };

        RSV_BASE_ENTRY rsvAlu1[ALU_ENT_NUM] {};
        RSV_BASE_ENTRY rsvAlu2[ALU_ENT_NUM] {};
        RSV_MUL_ENTRY rsvMul[MUL_ENT_NUM]   {};

        RSV_BRANCH_ENTRY rsvBranch[BRANCH_ENT_NUM]{};
        RSV_BASE_ENTRY   rsvLdSt[LDST_ENT_NUM]    {};


        /**
         * register architecture
         ***/

        struct MPFT_STATE{
            bool valids  [SPECTAG_LEN]{};
            bool fixTable[SPECTAG_LEN][SPECTAG_LEN]{};

            bool compare(const MPFT_STATE& rhs) const;
        } mpft;

        struct TAGGEN_STATE{
            ull brdepth = 0;
            ull tagReg  = 0;

            bool compare(const TAGGEN_STATE& rhs) const;
        }tagGen;

        struct ARF_STATE{
            //// the last one is master
            bool busy[SPECTAG_LEN+1][REG_NUM]{};
            ull  rename[SPECTAG_LEN+1][REG_NUM]{};

            bool compare(const ARF_STATE& rhs) const;

        }arf;

        struct RRF_STATE{
            bool busy[RRF_NUM]{};
            ull  data[RRF_NUM]{};

            bool compare(const RRF_STATE& rhs) const;
        }rrf;

        /**
         * execute unit
         */
        struct EXEC_ALU_STATE{
            pipStat st = IDLE;
            RSV_BASE_ENTRY entry{};

            bool compare(const EXEC_ALU_STATE& rhs) const;
        } exec_alu0, exec_alu1;

        struct EXEC_MUL_STATE{
            pipStat st = IDLE;
            RSV_MUL_ENTRY entry {};

            bool compare(const EXEC_MUL_STATE& rhs) const;
        } exec_mul;

        struct EXEC_BRANCH_STATE{
            pipStat st = IDLE;
            RSV_BRANCH_ENTRY entry{};

            bool compare(const EXEC_BRANCH_STATE& rhs) const;
        } exec_branch;

        struct EXEC_LDST_STATE{
            pipStat st = IDLE;
            RSV_BASE_ENTRY entry{};
            ull rrftag    = 0;
            ull rdUse     = 0;
            ull spec      = 0;
            ull specTag   = 0;
            ull stBufData = 0;
            ull stBufHit  = 0;

            bool compare(const EXEC_LDST_STATE& rhs) const;
        }exec_ldst;

        ////// commit stage


        ////// store buffer stage


        SimState(){

            /////// set name for all rsv
            for (int i = 0; i < ALU_ENT_NUM; i++){
                rsvAlu1[i].name = "RSV_ALU1_" + std::to_string(i);
                rsvAlu2[i].name = "RSV_ALU2_" + std::to_string(i);
            }
            for (int i = 0; i < MUL_ENT_NUM; i++){
                rsvMul[i].name = "RSV_MUL_" + std::to_string(i);
            }
            for (int i = 0; i < BRANCH_ENT_NUM; i++){
                rsvBranch[i].name = "RSV_BRANCH_" + std::to_string(i);
            }
            for (int i = 0; i < LDST_ENT_NUM; i++){
                rsvLdSt[i].name = "RSV_LDST_" + std::to_string(i);
            }
        }

        virtual void recruitValue() = 0;

        void printSlotWindow();

        void compare(SimState& rhs);

        bool isStateShouldCheckFurther(pipStat ps);
        
    };
    
}

#endif //SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H
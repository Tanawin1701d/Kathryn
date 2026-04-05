//
// Created by tanawin on 22/12/25.
//

#ifndef SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H
#define SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H

#include "util/fileWriter/slotWriter/slotWriterBase.h"
#include "kathryn.h"
#include "simState.h"
#include "example/o3/core/mpft.h"
#include "example/o3/core/parameter.h"
#include "example/o3/simShare/recPipStage.h"


namespace kathryn::o3{

    enum pipStat {PS_IDLE, PS_STALL, PS_RUNNING};

    inline const char* pipStatToString(pipStat st) {
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
            bool misPred  = false;
            bool succPred = false;
        } bcState, bcPrev;



        struct Fetch{
            pipStat st = PS_IDLE;
            ull     pc = 0;
            bool compare(const Fetch& rhs) const;
            void printSlot(SlotWriterBase& writer);


        } fetch;

        struct Decode{
            pipStat st       = PS_IDLE;
            ull     inst1    = 0;
            ull     invalid2 = 0;
            ull     inst2    = 0;
            ull     pc       = 0;
            ull     npc      = 0;
            /** not compare*/
            bool isGenable = false;

            bool compare(const Decode& rhs) const;
            void printSlot(SlotWriterBase& writer);

        } decode;

        struct DispInstr{
            //// TODO check predict cond mul use case
            int idx       = 0;
            ull invalid   = 0;
            ull immType  = 0;
            ull aluOp     = 0;
            ull rsEnt     = 0;
            ull isBranch  = 0;
            ull predAddr = 0;
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
            void printSlot(SlotWriterBase& writer);
        };

        struct Dispatch{
            pipStat st = PS_IDLE;
            ///// shared signal
            ull pc        = 0;
            ull desEqSrc1 = 0;
            ull desEqSrc2 = 0;
            ///// individual instruction
            DispInstr dp1 {0};
            DispInstr dp2 {1};
            /** not compare*/
            bool isAluRsvAllocatable = false;
            bool isBranchRsvAllocatable = false;
            bool isRenamable = false;

            bool compare(const Dispatch& rhs) const;
            void printSlot(SlotWriterBase& writer);

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
            ull aluOp  = 0; //////  mul and ldst discard it
            ull specBit = 0;
            ull spectag = 0;

            ull src1     = 0, src2     = 0;
            ull src1_sel = 0, src2_sel = 0; ///// ldst discards it
            ull valid1   = 0, valid2   = 0;

            bool compare(const RSV_BASE_ENTRY& rhs) const;
            void printSlot(SlotWriterBase& writer, REC_PIP_STAGE rps);
            void printDetailedSlot(SlotWriterBase& writer,
                                   REC_PIP_STAGE rps,
                                   const COMMIT_STATE& commitState);
        };


        struct RSV_BRANCH_ENTRY: RSV_BASE_ENTRY{
            ull immBr = 0;
            ull praddr = 0;
            ull opcode = 0;

            bool compare(const RSV_BRANCH_ENTRY& rhs) const;
            void printSlot(SlotWriterBase& writer, REC_PIP_STAGE rps);
            void printDetailedSlot(SlotWriterBase& writer,
                                   REC_PIP_STAGE rps,
                                   const COMMIT_STATE& commitState);
        };

        struct RSV_MUL_ENTRY: RSV_BASE_ENTRY{
            ull src1_signed = 0;
            ull src2_signed = 0;
            ull selLohi    = 0;

            bool compare(const RSV_MUL_ENTRY& rhs) const;
            void printSlot(SlotWriterBase& writer, REC_PIP_STAGE rps);
            void printDetailedSlot(SlotWriterBase& writer,
                                   REC_PIP_STAGE rps,
                                   const COMMIT_STATE& commitState);
        };

        RSV_BASE_ENTRY rsvAlu1[ALU_ENT_NUM] {};
        RSV_BASE_ENTRY rsvAlu2[ALU_ENT_NUM] {};
        RSV_MUL_ENTRY  rsvMul[MUL_ENT_NUM]   {};

        RSV_BRANCH_ENTRY rsvBranch[BRANCH_ENT_NUM]{};
        RSV_BASE_ENTRY   rsvLdSt[LDST_ENT_NUM]    {};

        ////////// do not compare
        pipStat stIssueAlu1   = PS_IDLE; ull idxIssueAlu1   = 0;
        pipStat stIssueAlu2   = PS_IDLE; ull idxIssueAlu2   = 0;
        pipStat stIssueMul    = PS_IDLE; ull idxIssueMul    = 0;
        pipStat stIssueBranch = PS_IDLE; ull idxIssueBranch = 0;
        pipStat stIssueLdst   = PS_IDLE; ull idxIssueLdst   = 0;

        /**
         * execute unit
         */
        struct EXEC_ALU_STATE{
            pipStat st = PS_IDLE;
            RSV_BASE_ENTRY entry{};

            bool compare(const EXEC_ALU_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState);
        } execAlu1, execAlu2;

        struct EXEC_MUL_STATE{
            pipStat st = PS_IDLE;
            RSV_MUL_ENTRY entry {};

            bool compare(const EXEC_MUL_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState);
        } execMul;

        struct EXEC_BRANCH_STATE{
            pipStat st = PS_IDLE;
            RSV_BRANCH_ENTRY entry{};

            //// not compare

            bool compare(const EXEC_BRANCH_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState, BC bc);
        } execBranch;

        struct EXEC_LDST_STATE{
            pipStat st1 = PS_IDLE;
            pipStat st2 = PS_IDLE;
            RSV_BASE_ENTRY entry{};
            ull     effAddr = 0;


            ull rrftag    = 0;
            ull rdUse     = 0;
            ull spec      = 0;
            ull specTag   = 0;
            ull stBufData = 0;
            ull stBufHit  = 0;
            ull loadData  = 0;

            bool compare(const EXEC_LDST_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState);
        }execLdst;

        ////// commit stage

        struct COMMIT_ENTRY{
            int idx       = 0;
            ull wbFin     = 0;
            ull storeBit  = 0;
            ull rdUse     = 0;
            ull rdIdx     = 0;

            bool compare(const COMMIT_ENTRY& rhs) const;
            void printSlot(SlotWriterBase& writer);
        };

        struct COMMIT_STATE{
            ull  comPtr     = 0;
            bool com1Status = false;
            bool com2Status = false;
            COMMIT_ENTRY comEntries[RRF_NUM]{};
            //////// do not compare this section
            bool isPrevCycleDp1 = false;
            bool isPrevCycleDp2 = false;
            ull  dpPointer = 0;

            COMMIT_STATE(){
                for (int i = 0; i < RRF_NUM; i++){
                    comEntries[i].idx = i;
                }
            }

            bool compare(const COMMIT_STATE& rhs, ull reqPtr) const;
            void printSlot(SlotWriterBase& writer);
        } rob;

        ////// store buffer stage

        struct STORE_BUF_ENTRY{
            int  idx        = 0;
            bool busy       = false;
            ull  complete   = 0;
            ull  spec       = 0;
            ull  specTag    = 0;
            ull  memAddr   = 0;
            ull  memData   = 0;

            bool compare(const STORE_BUF_ENTRY& rhs) const;
            void printSlot(SlotWriterBase& writer);
        };

        struct STORE_BUF_STATE{
            ull finPtr = 0;
            ull comPtr = 0;
            ull retPtr = 0;
            STORE_BUF_ENTRY entries[STBUF_ENT_NUM]{};
            ///// not compare
            ull  nb1    = 0;
            ull  ne1    = 0;
            ull  nb0    = 0;
            bool fullNext = false;
            bool emptyNext = false;

            STORE_BUF_STATE(){
                for (int i = 0; i < STBUF_ENT_NUM; i++){
                    entries[i].idx = i;
                }
            }


            bool compare(const STORE_BUF_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer);
        } stbuf;

        /**
         * register architecture
         ***/

        struct MPFT_STATE{
            bool valids  [SPECTAG_LEN]{};
            bool fixTable[SPECTAG_LEN][SPECTAG_LEN]{};

            bool compare(const MPFT_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer);
        } mpft;

        struct TAGGEN_STATE{
            ull brdepth = 0;
            ull tagReg  = 0;

            bool compare(const TAGGEN_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer);
        }tagGen;

        struct ARF_STATE{
            //// the last one is master
            bool busy  [SPECTAG_LEN+1][REG_NUM]{};
            ull  rename[SPECTAG_LEN+1][REG_NUM]{};

            bool compare(const ARF_STATE& rhs) const;
            void printSlot(SlotWriterBase& writer, BC bcPrev);

        }arf;

        struct RRF_STATE{
            bool busy[RRF_NUM]{};
            ull  data[RRF_NUM]{};

            ull freenum      = 0;
            ull reqPtr       = 0;
            ull nextRrfCycle = 0;

            /**
             * check only on comPtr to reqPtr)
             */
            bool compare(const RRF_STATE& rhs, ull comPtr) const;
            void printSlot(SlotWriterBase& writer);
        }rrf;


        SimState(){

            /////// set name for all rsv
            for (int i = 0; i < ALU_ENT_NUM; i++){
                rsvAlu1[i].name = "RSV_ALU1_" + std::toString(i);
                rsvAlu1[i].idx  = i;
                rsvAlu2[i].name = "RSV_ALU2_" + std::toString(i);
                rsvAlu2[i].idx  = i;
            }
            for (int i = 0; i < MUL_ENT_NUM; i++){
                rsvMul[i].name = "RSV_MUL_" + std::toString(i);
                rsvMul[i].idx  = i;
            }
            for (int i = 0; i < BRANCH_ENT_NUM; i++){
                rsvBranch[i].name = "RSV_BRANCH_" + std::toString(i);
                rsvBranch[i].idx  = i;
            }
            for (int i = 0; i < LDST_ENT_NUM; i++){
                rsvLdSt[i].name = "RSV_LDST_" + std::toString(i);
                rsvLdSt[i].idx  = i;
            }
        }

        virtual void recruitValue()     = 0;
        virtual void recruitNextCycle() = 0;

        bool compare(SimState& rhs) const;
        virtual void printSlotWindow(SlotWriterBase& writer);

        bool isStateShouldCheckFurther(pipStat ps);
        
    };
    
}

#endif //SRC_EXAMPLE_O3_SIMCOMPARE_SIMSTATE_H
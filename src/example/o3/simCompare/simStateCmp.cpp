//
// Created by tanawin on 22/12/25.
//

#include "simState.h"

namespace kathryn::o3{

    void printStateMisMatch(const std::string& stageName, pipStat lhs, pipStat rhs){
        std::cout << stageName << ": Mismatch on simState" << std::endl;
        std::cout << "lhs: " << pipStatToString(lhs)
                  << " rhs: " << pipStatToString(rhs) << std::endl;
    }

    bool checkAndPrintSimValueUll(ull lhs, ull rhs,
                               const std::string& stageName,
                               const std::string& simName){
        if (lhs != rhs){
            std::cout << stageName << "with simType "<< simName
                      << ": LHS: " << lhs << " RHS: " << rhs << std::endl;
            return false;
        }
        return true;
    }

    bool checkAndPrintSimValueBool(bool lhs, bool rhs,
                               const std::string& stageName,
                               const std::string& simName){
        if (lhs != rhs){
            std::cout << stageName << "with simType "<< simName
                      << ": LHS: " << lhs << " RHS: " << rhs << std::endl;
            return false;
        }
        return true;
    }




    bool SimState::isStateShouldCheckFurther(pipStat ps){ return ps == RUNNING;}

    /**
     * fetch check
     ***/

    bool SimState::Fetch::compare(const Fetch& rhs) const{

        if (st != rhs.st){
            printStateMisMatch("fetch stage", st, rhs.st);
            return false;
        }
        bool compareResult = true;
        compareResult &= checkAndPrintSimValueUll(pc, rhs.pc, "fetch", "pc");
        return compareResult;
    }

    /**
     *
     * decode
     *
     */
    bool SimState::Decode::compare(const Decode& rhs) const{

        if (st != rhs.st){
            printStateMisMatch("decode stage", st, rhs.st);
            return false;
        }

        bool compareResult = true;
        compareResult &= checkAndPrintSimValueUll(inst1 , rhs.inst1 , "decode", "inst1" );
        compareResult &= checkAndPrintSimValueUll(invalid2 , rhs.invalid2 , "decode", "invalid2" );

        if (invalid2 != 0){
            compareResult &= checkAndPrintSimValueUll(inst2 , rhs.inst2 , "decode", "inst2" );
        }

        compareResult &= checkAndPrintSimValueUll(pc    , rhs.pc    , "decode", "pc" );
        compareResult &= checkAndPrintSimValueUll(npc   , rhs.npc   , "decode", "npc");
        compareResult &= checkAndPrintSimValueUll(bhr   , rhs.bhr   , "decode", "bhr");

        return compareResult;

    }

    /**
     *
     *  dispatch
     */
    bool SimState::DispInstr::compare(const DispInstr& rhs) const{

        if (invalid != rhs.invalid){
            checkAndPrintSimValueUll(invalid  , rhs.invalid , "dispatch", "invalid  ");
            return false;
        }
        if (invalid){
            return true;
        }
        bool compareResult = true;
        compareResult &= checkAndPrintSimValueUll(imm_type  , rhs.imm_type , "dispatch", "imm_type  ");
        compareResult &= checkAndPrintSimValueUll(aluOp     , rhs.aluOp    , "dispatch", "aluOp     ");
        compareResult &= checkAndPrintSimValueUll(rsEnt     , rhs.rsEnt    , "dispatch", "rsEnt     ");
        compareResult &= checkAndPrintSimValueUll(isBranch  , rhs.isBranch , "dispatch", "isBranch  ");
        compareResult &= checkAndPrintSimValueUll(pred_addr , rhs.pred_addr, "dispatch", "pred_addr ");
        compareResult &= checkAndPrintSimValueUll(spec      , rhs.spec     , "dispatch", "spec      ");
        compareResult &= checkAndPrintSimValueUll(specTag   , rhs.specTag  , "dispatch", "specTag   ");
        compareResult &= checkAndPrintSimValueUll(rdIdx     , rhs.rdIdx    , "dispatch", "rdIdx     ");
        compareResult &= checkAndPrintSimValueUll(rdUse     , rhs.rdUse    , "dispatch", "rdUse     ");
        compareResult &= checkAndPrintSimValueUll(rsIdx_1   , rhs.rsIdx_1  , "dispatch", "rsIdx_1   ");
        compareResult &= checkAndPrintSimValueUll(rsSel_1   , rhs.rsSel_1  , "dispatch", "rsSel_1   ");
        compareResult &= checkAndPrintSimValueUll(rsUse_1   , rhs.rsUse_1  , "dispatch", "rsUse_1   ");
        compareResult &= checkAndPrintSimValueUll(rsIdx_2   , rhs.rsIdx_2  , "dispatch", "rsIdx_2   ");
        compareResult &= checkAndPrintSimValueUll(rsSel_2   , rhs.rsSel_2  , "dispatch", "rsSel_2   ");
        compareResult &= checkAndPrintSimValueUll(rsUse_2   , rhs.rsUse_2  , "dispatch", "rsUse_2   ");

        return compareResult;
    }

    bool SimState::Dispatch::compare(const Dispatch& rhs) const{

        if (st != rhs.st){
            printStateMisMatch("dispatch stage", st, rhs.st);
            return false;
        }

        bool compareResult = true;
        compareResult &= checkAndPrintSimValueUll(pc, rhs.pc, "dispatch", "pc");
        compareResult &= checkAndPrintSimValueUll(desEqSrc1, rhs.desEqSrc1, "dispatch", "desEqSrc1");
        compareResult &= checkAndPrintSimValueUll(desEqSrc2, rhs.desEqSrc2, "dispatch", "desEqSrc2");
        compareResult &= dp0.compare(rhs.dp0);
        compareResult &= dp1.compare(rhs.dp1);

        return compareResult;
    }

    //////// RSV_BASE_ENTRY


    bool SimState::RSV_BASE_ENTRY::compare(const RSV_BASE_ENTRY& rhs) const{

        if (!checkAndPrintSimValueUll(busy, rhs.busy, "RSV_BASE_ENTRY", "busy")){
            return false;
        }
        bool compareResult = true;
        if (busy == 0){ return compareResult; }



        compareResult &= checkAndPrintSimValueUll(sortbit, rhs.sortbit, name, "sortbit");
        compareResult &= checkAndPrintSimValueUll(pc     , rhs.pc     , name, "pc"     );
        compareResult &= checkAndPrintSimValueUll(imm    , rhs.imm    , name, "imm"    );
        compareResult &= checkAndPrintSimValueUll(rrftag , rhs.rrftag , name, "rrftag" );
        compareResult &= checkAndPrintSimValueUll(dstval , rhs.dstval , name, "dstval" );
        compareResult &= checkAndPrintSimValueUll(alu_op , rhs.alu_op , name, "alu_op" );
        compareResult &= checkAndPrintSimValueUll(spectag, rhs.spectag, name, "spectag");

        if (valid1 && src1_sel == SRC_A_RS1){
            compareResult &= checkAndPrintSimValueUll(src1, rhs.src1, name, "src1");
        }
        if (valid2 && src2_sel == SRC_B_RS2){
            compareResult &= checkAndPrintSimValueUll(src2, rhs.src2, name, "src2");
        }
        return compareResult;
    }

    //////// RSV_BRANCH_ENTRY

    bool SimState::RSV_BRANCH_ENTRY::compare(const RSV_BRANCH_ENTRY& rhs) const{

        bool compareResult = RSV_BASE_ENTRY::compare(rhs);

        if (busy == 0){ return compareResult; }

        compareResult &= checkAndPrintSimValueUll(bhr   , rhs.bhr   , name, "bhr   ");
        compareResult &= checkAndPrintSimValueUll(prcond, rhs.prcond, name, "prcond");
        compareResult &= checkAndPrintSimValueUll(praddr, rhs.praddr, name, "praddr");
        compareResult &= checkAndPrintSimValueUll(opcode, rhs.opcode, name, "opcode");

        return compareResult;

    }

    bool SimState::RSV_MUL_ENTRY::compare(const RSV_MUL_ENTRY& rhs) const{

        bool compareResult = RSV_BASE_ENTRY::compare(rhs);

        if (busy == 0){ return compareResult; }

        compareResult &= checkAndPrintSimValueUll(src1_signed, rhs.src1_signed, name, "src1_signed");
        compareResult &= checkAndPrintSimValueUll(src2_signed, rhs.src2_signed, name, "src2_signed");
        compareResult &= checkAndPrintSimValueUll(sel_lohi   , rhs.sel_lohi   , name, "sel_lohi");

        return compareResult;

    }

    /**
     *
     * REG architecture
     *
     */
    bool  SimState::MPFT_STATE::compare(const MPFT_STATE& rhs) const{

        bool compareResult = true;
        for (int idx = 0; idx < SPECTAG_LEN; idx++){
                compareResult &= checkAndPrintSimValueBool(valids[idx], rhs.valids[idx], "MPFT", "valids" + std::to_string(idx));
            if (valids[idx]){
                for (int fixIdx = 0; fixIdx < SPECTAG_LEN; fixIdx++){
                    compareResult &= checkAndPrintSimValueBool(fixTable[idx][fixIdx], rhs.fixTable[idx][fixIdx],
                                                               "MPFT", "fixTable idx: " + std::to_string(idx) +
                                                               " fixIdx: " + std::to_string(fixIdx));
                }
            }

        }
        return compareResult;
    }

    bool SimState::TAGGEN_STATE::compare(const TAGGEN_STATE& rhs) const{
        return checkAndPrintSimValueUll(brdepth, rhs.brdepth, "TAGGEN", "brdepth") &&
               checkAndPrintSimValueUll(tagReg , rhs.tagReg , "TAGGEN", "tagReg" );
    }


    bool SimState::ARF_STATE::compare(const ARF_STATE& rhs) const{

        bool compareResult = true;
        for (int idx = 0; idx <= SPECTAG_LEN; idx++){

            for (int archIdx = 0; archIdx < REG_NUM; archIdx++){
                compareResult &= checkAndPrintSimValueBool(busy[idx][archIdx],
                                                          rhs.busy[idx][archIdx],
                                                          "ARF",
                                                          "busy spec: " + std::to_string(idx) +
                                                          " archIdx: " + std::to_string(archIdx));
                if (busy[idx][archIdx]){
                    compareResult &= checkAndPrintSimValueUll(rename[idx][archIdx],
                                                              rhs.rename[idx][archIdx], "ARF",
                                                                "archRem spec: " + std::to_string(idx) +
                                                                "archIdx: " + std::to_string(archIdx)
                    );
                }
            }
        }
        return compareResult;
    }

    bool SimState::RRF_STATE::compare(const RRF_STATE& rhs) const{
        bool compareResult = true;
        for (int idx = 0; idx < RRF_NUM; idx++){
            compareResult &= checkAndPrintSimValueBool(busy[idx], rhs.busy[idx], "RRF", "valids" + std::to_string(idx));
            if (busy[idx]){
                compareResult &= checkAndPrintSimValueUll(data[idx], rhs.data[idx],
                                                           "RRF", "phyIdx: " + std::to_string(idx));
            }
        }
        return compareResult;
    }

    /**
     * exec unit
     *
     ***/

    bool SimState::EXEC_ALU_STATE::compare(const EXEC_ALU_STATE& rhs) const{
        if (st != rhs.st){
            printStateMisMatch("exec ALU stage", st, rhs.st);
            return false;
        }
        return entry.compare(rhs.entry);
    }

    bool SimState::EXEC_MUL_STATE::compare(const EXEC_MUL_STATE& rhs) const{
        if (st != rhs.st){
            printStateMisMatch("exec mul stage", st, rhs.st);
            return false;
        }
        return entry.compare(rhs.entry);
    }

    bool SimState::EXEC_BRANCH_STATE::compare(const EXEC_BRANCH_STATE& rhs) const{
        if (st != rhs.st){
            printStateMisMatch("exec branch stage", st, rhs.st);
            return false;
        }
        return entry.compare(rhs.entry);
    }

    bool SimState::EXEC_LDST_STATE::compare(const EXEC_LDST_STATE& rhs) const{
        if (st != rhs.st){
            printStateMisMatch("exec LDST stage", st, rhs.st);
            return false;
        }
        bool compareResult =  entry.compare(rhs.entry);

        compareResult &= checkAndPrintSimValueUll(rrftag, rhs.rrftag, "exec LDST", "rrftag");
        compareResult &= checkAndPrintSimValueUll(rdUse, rhs.rdUse, "exec LDST", "rdUse");
        compareResult &= checkAndPrintSimValueUll(spec, rhs.spec, "exec LDST", "spec");
        compareResult &= checkAndPrintSimValueUll(specTag, rhs.specTag, "exec LDST", "specTag");
        compareResult &= checkAndPrintSimValueUll(stBufData, rhs.stBufData, "exec LDST", "stBufData");
        compareResult &= checkAndPrintSimValueUll(stBufHit, rhs.stBufHit, "exec LDST", "stBufHit");

        return compareResult;
    }









}
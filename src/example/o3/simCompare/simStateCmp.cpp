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




    bool SimState::isStateShouldCheckFurther(pipStat ps){ return ps == PS_RUNNING;}

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
        compareResult &= dp1.compare(rhs.dp1);
        compareResult &= dp2.compare(rhs.dp2);

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

        compareResult &= checkAndPrintSimValueUll(imm_br, rhs.imm_br, name, "immbr");
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
        bool compareResult = true;
        if (st1 == rhs.st1){
            compareResult &= entry.compare(rhs.entry);
        }else{
            printStateMisMatch("exec LDST stage", st1, rhs.st1);
            compareResult = false;
        }

        if (st2 == rhs.st2){
            if (st2 == PS_RUNNING){
                compareResult &= checkAndPrintSimValueUll(rrftag   , rhs.rrftag   , "exec LDST", "rrftag"   );
                compareResult &= checkAndPrintSimValueUll(rdUse    , rhs.rdUse    , "exec LDST", "rdUse"    );
                compareResult &= checkAndPrintSimValueUll(spec     , rhs.spec     , "exec LDST", "spec"     );
                compareResult &= checkAndPrintSimValueUll(specTag  , rhs.specTag  , "exec LDST", "specTag"  );
                compareResult &= checkAndPrintSimValueUll(stBufData, rhs.stBufData, "exec LDST", "stBufData");
                compareResult &= checkAndPrintSimValueUll(stBufHit , rhs.stBufHit , "exec LDST", "stBufHit" );
            }

        }else{
            printStateMisMatch("exec1 LDST stage", st1, rhs.st1);
            compareResult = false;

        }




        return compareResult;
    }


    /**
     *
     *  commit stage
     */
    bool SimState::COMMIT_ENTRY::compare(const COMMIT_ENTRY& rhs) const{

        bool compareResult = true;

        compareResult &= checkAndPrintSimValueUll(wbFin   , rhs.wbFin   , "COMMIT entry " + std::to_string(idx), "wbFin");
        if (wbFin == 0){
            return compareResult;
        }
        compareResult &= checkAndPrintSimValueUll(storeBit, rhs.storeBit, "COMMIT entry " + std::to_string(idx), "storeBit");
        compareResult &= checkAndPrintSimValueUll(rdUse   , rhs.rdUse   , "COMMIT entry " + std::to_string(idx), "rdUse");
        compareResult &= checkAndPrintSimValueUll(rdIdx   , rhs.rdIdx   , "COMMIT entry " + std::to_string(idx), "rdIdx");

        return compareResult;

    }

    bool SimState::COMMIT_STATE::compare(const COMMIT_STATE& rhs) const{
        bool compareResult = true;
        compareResult &= checkAndPrintSimValueUll(comPtr    , rhs.comPtr    , "COMMIT", "comPtr"    );
        compareResult &= checkAndPrintSimValueBool(com1Status, rhs.com1Status, "COMMIT", "com1Status");
        compareResult &= checkAndPrintSimValueBool(com2Status, rhs.com2Status, "COMMIT", "com2Status");
        for (int rrfIdx = 0; rrfIdx < RRF_NUM; rrfIdx++){
            compareResult &= comEntries[rrfIdx].compare(rhs.comEntries[rrfIdx]);
        }

        return compareResult;
    }

    /**
     * store buffer state
     */

    bool SimState::STORE_BUF_ENTRY::compare(const STORE_BUF_ENTRY& rhs) const{

        if (!checkAndPrintSimValueBool(busy    , rhs.busy,     "STORE_BUF", "busy")){
            return false;
        }

        if (busy == 0){ return true; }

        return checkAndPrintSimValueUll (complete, rhs.complete, "STORE_BUF", "complete") &&
               checkAndPrintSimValueUll (spec    , rhs.spec,     "STORE_BUF", "spec") &&
               checkAndPrintSimValueUll (specTag , rhs.specTag,  "STORE_BUF", "specTag") &&
               checkAndPrintSimValueUll (mem_addr, rhs.mem_addr, "STORE_BUF", "mem_addr") &&
               checkAndPrintSimValueUll (mem_data, rhs.mem_data, "STORE_BUF", "mem_data");
    }

    bool SimState::STORE_BUF_STATE::compare(const STORE_BUF_STATE& rhs) const{
        bool compareResult = true;
        compareResult &= checkAndPrintSimValueUll(finPtr, rhs.finPtr, "STORE_BUF", "finPtr");
        compareResult &= checkAndPrintSimValueUll(comPtr, rhs.comPtr, "STORE_BUF", "comPtr");
        compareResult &= checkAndPrintSimValueUll(retPtr, rhs.retPtr, "STORE_BUF", "retPtr");
        for (int idx = 0; idx < STBUF_ENT_NUM; idx++){
            compareResult &= entries[idx].compare(rhs.entries[idx]);
        }
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
        for (int tableIdx = 0; tableIdx <= SPECTAG_LEN; tableIdx++){

            for (int archIdx = 0; archIdx < REG_NUM; archIdx++){
                compareResult &= checkAndPrintSimValueBool(busy[tableIdx][archIdx],
                                                          rhs.busy[tableIdx][archIdx],
                                                          "ARF",
                                                          "busy spec: " + std::to_string(tableIdx) +
                                                          " archIdx: " + std::to_string(archIdx));
                if (busy[tableIdx][archIdx]){
                    compareResult &= checkAndPrintSimValueUll(rename[tableIdx][archIdx],
                                                              rhs.rename[tableIdx][archIdx], "ARF",
                                                              "archRem spec: " + std::to_string(tableIdx) +
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

        compareResult &= checkAndPrintSimValueUll(freenum     , rhs.freenum     , "RRF", "freenum");
        compareResult &= checkAndPrintSimValueUll(reqPtr      , rhs.reqPtr      , "RRF", "reqPtr");
        compareResult &= checkAndPrintSimValueUll(nextRrfCycle, rhs.nextRrfCycle, "RRF", "nextRrfCycle");

        return compareResult;
    }

    bool SimState::compare(SimState& rhs) const{

        bool compareResult = true;

        compareResult &= fetch.compare(rhs.fetch);
        compareResult &= decode.compare(rhs.decode);
        compareResult &= dispatch.compare(rhs.dispatch);

        ////// rsv compare
        for (int rsvIdx = 0; rsvIdx < ALU_ENT_NUM; rsvIdx++){
            compareResult &= rsvAlu1[rsvIdx].compare(rhs.rsvAlu1[rsvIdx]);
        }
        for (int rsvIdx = 0; rsvIdx < ALU_ENT_NUM; rsvIdx++){
            compareResult &= rsvAlu2[rsvIdx].compare(rhs.rsvAlu2[rsvIdx]);
        }
        for (int rsvIdx = 0; rsvIdx < MUL_ENT_NUM; rsvIdx++){
            compareResult &= rsvMul[rsvIdx].compare(rhs.rsvMul[rsvIdx]);
        }
        for (int rsvIdx = 0; rsvIdx < BRANCH_ENT_NUM; rsvIdx++){
            compareResult &= rsvBranch[rsvIdx].compare(rhs.rsvBranch[rsvIdx]);
        }
        for (int rsvIdx = 0; rsvIdx < LDST_ENT_NUM; rsvIdx++){
            compareResult &= rsvLdSt[rsvIdx].compare(rhs.rsvLdSt[rsvIdx]);
        }

        compareResult &= exec_alu1.compare(rhs.exec_alu1);
        compareResult &= exec_alu2.compare(rhs.exec_alu2);
        compareResult &= exec_mul.compare(rhs.exec_mul);
        compareResult &= exec_branch.compare(rhs.exec_branch);
        compareResult &= exec_ldst.compare(rhs.exec_ldst);

        compareResult &= rob.compare(rhs.rob);
        compareResult &= stbuf.compare(rhs.stbuf);

        //// register architecture
        compareResult &= mpft.compare(rhs.mpft);
        compareResult &= tagGen.compare(rhs.tagGen);
        compareResult &= arf.compare(rhs.arf);
        compareResult &= rrf.compare(rhs.rrf);

        return compareResult;
    }

}
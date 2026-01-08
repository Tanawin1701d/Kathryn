//
// Created by tanawin on 24/12/25.
//

#include "simState.h"

namespace kathryn::o3{


    /**
     * shared functionality
     *
     */

    std::string lookupImmType(ull key){
        static const std::map<ull, std::string> immTypeMap = {
            {0b00, "IMM_I"}, {0b01, "IMM_S"},
            {0b10, "IMM_U"}, {0b11, "IMM_J"}
        };
        auto it = immTypeMap.find(key);
        return (it != immTypeMap.end()) ? it->second : "UNKNOWN";
    }

    std::string lookupRsvType(ull key){
        static const std::map<ull, std::string> rsvTypeMap = {
            {RS_ENT_ALU, "ALU"}, {RS_ENT_BRANCH, "BRANCH"},
            {RS_ENT_JAL, "JAL"}, {RS_ENT_JALR, "JALR"},
            {RS_ENT_MUL, "MUL"}, {RS_ENT_DIV, "DIV"},
            {RS_ENT_LDST, "LDST"}
        };
        auto it = rsvTypeMap.find(key);
        return (it != rsvTypeMap.end()) ? it->second : "UNKNOWN";
    }

    std::string lookupSrcASel(ull key){
        static const std::map<ull, std::string> srcASelMap = {
            {0, "RS1"}, {1, "PC"}, {2, "ZERO"}
        };
        auto it = srcASelMap.find(key);
        return (it != srcASelMap.end()) ? it->second : "UNKNOWN";
    }

    std::string lookupSrcBSel(ull key){
        static const std::map<ull, std::string> srcBSelMap = {
            {0, "RS2"}, {1, "IMM"}, {2, "FOUR"}, {3, "ZERO"}
        };
        auto it = srcBSelMap.find(key);
        return (it != srcBSelMap.end()) ? it->second : "UNKNOWN";
    }

    

    bool writeSlotIfRunning(REC_PIP_STAGE rps, pipStat st, SlotWriterBase& writer){

        writer.addSlotVal(rps, pipStatToString(st));
        return st == PS_RUNNING;
    }

    void writeIssue(const std::string& stageName,
                    SlotWriterBase& writer,
                    pipStat st,
                    ull issueIdx,
                    bool isOhIdx){

        writer.addSlotVal(RPS_ISSUE, "-----> "+ stageName + "ISSUE Stage");
        if (!writeSlotIfRunning(RPS_ISSUE, st, writer)){
            return;
        }

        if (isOhIdx){
            writer.addSlotVal(RPS_ISSUE, "issue idx: " + std::to_string(log2(issueIdx)));
        }else{
            writer.addSlotVal(RPS_ISSUE, "issue idx: " + std::to_string(issueIdx));

        }


    }

    std::string translateInstrToOpCode(ull rawInstr){
        std::map<ull, std::string> decMap = {
            {0b00'000'11, "LOAD"   }, {0b01'000'11, "STORE"  },
            {0b00'011'11, "MISCMEM"}, {0b11'000'11, "BRANCH" },
            {0b11'001'11, "JALR"   }, {0b11'011'11, "JAL"    },
            {0b00'100'11, "OP_IMM" }, {0b01'100'11, "OP"     },
            {0b00'101'11, "AUIPC"  }, {0b01'101'11, "LUI"    },
            {0b11'100'11, "SYSTEM" },
        };
        ull opMaskBit = (1 << 7) - 1;
        ull op = rawInstr & opMaskBit;
        return (decMap.find(op) != decMap.end()) ? decMap[op] : "UNKNOWN";
    }


    std::string translateAluOpToOp(ull aluOpIdx){
        std::map<ull, std::string> aluOpMap = {
            { 0, "ADD"} , { 1, "SLL"} , { 4, "XOR"},
            { 6, "OR"}  , { 7, "AND"} , { 5, "SRL"},
            { 8, "SEQ"} , { 9, "SNE"} , {10, "SUB"},
            {11, "SRA"} , {12, "SLT"} , {13, "SGE"},
            {14, "SLTU"}, {15, "SGEU"}
        };
        return (aluOpMap.find(aluOpIdx) != aluOpMap.end()) ? aluOpMap[aluOpIdx] : "UNKNOWN";
    }

    /**
     * print
     */
    void SimState::Fetch::printSlot(SlotWriterBase& writer){

        if (!writeSlotIfRunning(RPS_FETCH, st, writer)){
            return;
        }
        writer.addSlotVal(RPS_FETCH, "pc : " + cvtNum2HexStr(pc));

    }

    void SimState::Decode::printSlot(SlotWriterBase& writer){

        if (!writeSlotIfRunning(RPS_DECODE, st, writer)){
            return;
        }
        writer.addSlotVal(RPS_DECODE, "gennable "+ std::to_string(isGenable));
        writer.addSlotVal(RPS_DECODE, "PC" + cvtNum2HexStr(pc));
        writer.addSlotVal(RPS_DECODE, "NPC" + cvtNum2HexStr(npc));

        for (int i = 1; i <= 2; i++){
            writer.addSlotVal(RPS_DECODE, "- DEC " + std::to_string(i) + "-");
            ull sim_invalid = (i == 1) ? 0     : invalid2;
            ull sim_instr   = (i == 1) ? inst1 : inst2;

            if (sim_invalid){
                writer.addSlotVal(RPS_DECODE, "notValid");
            }else{
                std::string decStr = "valid OP: ";
                decStr += translateInstrToOpCode(sim_instr);
                writer.addSlotVal(RPS_DECODE, decStr);
            }
        }
    }

    void SimState::DispInstr::printSlot(SlotWriterBase& writer){

        if (invalid){
                writer.addSlotVal(RPS_DISPATCH, "notValid");
        }else{

            writer.addSlotVal(RPS_DISPATCH, "RS: " + lookupRsvType(rsEnt));
            writer.addSlotVal(RPS_DISPATCH, lookupImmType(imm_type));
            writer.addSlotVal(RPS_DISPATCH, "ALU: " + translateAluOpToOp(aluOp));

            writer.addSlotVal(RPS_DISPATCH, "isBr: " + std::to_string(isBranch) +
                                            "/isSp: " + std::to_string(spec) +
                                            "/spTag: " + cvtNum2BinStr(specTag));
            writer.addSlotVal(RPS_DISPATCH, "nextPc_" + cvtNum2HexStr(pred_addr));

            std::string rdUsage = (rdUse != 0) ? "(USE)" : "(UNUSED)";
            writer.addSlotVal(RPS_DISPATCH, "RD: "+ rdUsage + " /ArchIdx: " +  std::to_string(rdIdx));

            std::string r1Usage = (rsUse_1 != 0) ? "(USE)" : "(UNUSED)";
            std::string selStr = lookupSrcASel(rsSel_1);
            writer.addSlotVal(RPS_DISPATCH, "RS1: " + r1Usage + " /IDX:" + std::to_string(rsIdx_1) +
                                    " /SEL:" + selStr);

            std::string r2Usage = (rsUse_2 != 0) ? "(USE)" : "(UNUSED)";
            std::string selStr2 = lookupSrcBSel(rsSel_2);
            writer.addSlotVal(RPS_DISPATCH, "RS2: " + r2Usage + " /IDX:" + std::to_string(rsIdx_2) +
                                    " /SEL:" + selStr2);
        }

    }

    void SimState::Dispatch::printSlot(SlotWriterBase& writer){

        if (!writeSlotIfRunning(RPS_DISPATCH, st, writer)){
            return;
        }
        writer.addSlotVal(RPS_DISPATCH, "aluRsvAble: " + std::to_string(isAluRsvAllocatable));
        writer.addSlotVal(RPS_DISPATCH, "brRsvAble: " + std::to_string(isBranchRsvAllocatable));
        writer.addSlotVal(RPS_DISPATCH, "isRenam: " + std::to_string(isRenamable));
        writer.addSlotVal(RPS_DISPATCH, "PC: " + cvtNum2HexStr(pc));
        std::string internalDep = str("S1EqDes: ") + (desEqSrc1? "1 " : "0 ") +
                                  str("S2EqDes: ") + (desEqSrc2? "1 " : "0 ");
        writer.addSlotVal(RPS_DISPATCH, internalDep);

        writer.addSlotVal(RPS_DISPATCH, "---->instr0");
        dp1.printSlot(writer);
        writer.addSlotVal(RPS_DISPATCH, "---->instr1");
        dp2.printSlot(writer);
    }

    void SimState::RSV_BASE_ENTRY::printSlot(SlotWriterBase& writer, REC_PIP_STAGE rps){

        if (busy == 0){return;}

        std::string result0;
        result0 += std::to_string(idx) + "] ";

        result0 += "sb:" + std::string(sortbit ? "1" : "0") + " ";


        result0 += "pc:" + cvtNum2HexStr(pc) + " ";
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
        writer.addSlotVal(rps, result0);

        ////// result 2 speculative meta data
        std::string result1;
        result1 += "spec: " + std::to_string(specBit) + " spt:" + cvtNum2BinStr(spectag);

        writer.addSlotVal(rps, result1);


    }

    void SimState::RSV_BRANCH_ENTRY::printSlot(SlotWriterBase& writer, REC_PIP_STAGE rps){
        RSV_BASE_ENTRY::printSlot(writer, rps);
    }



    void SimState::RSV_MUL_ENTRY::printSlot(SlotWriterBase& writer, REC_PIP_STAGE rps){

        RSV_BASE_ENTRY::printSlot(writer, rps);
        if (busy == 0){return;}
        std::string r1s = (src1_signed) ? "s" : "u";
        std::string r2s = (src2_signed) ? "s" : "u";
        std::string hl  = (sel_lohi)    ? "h" : "l";
        writer.addSlotVal(rps,"mulReq: 1:" + r1s + " "
                         + "2:" + r2s + " "
                         + "sl:");
    }

    void SimState::RSV_BASE_ENTRY::printDetailedSlot(SlotWriterBase& writer,
                                                     REC_PIP_STAGE rps,
                                                     const COMMIT_STATE& commitState){
        writer.addSlotVal(rps, "PC: " + cvtNum2HexStr(pc));
        writer.addSlotVal(rps, "IMM: " + cvtNum2HexStr(imm));
        writer.addSlotVal(rps, "ALU Op: " + translateAluOpToOp(alu_op) +
                               "/Spec: " + std::to_string(specBit) +
                               "/SpecTag: " + cvtNum2BinStr(spectag));
        std::string sim_isRdUsed = dstval ? "(USE)" : "(UNUSED)";
        if (dstval){
            ull archIdx = commitState.comEntries[rrftag].rdIdx;
            writer.addSlotVal(rps, "RD phy: " + std::to_string(rrftag) + " arch: " + std::to_string(archIdx));
        }else{
            writer.addSlotVal(rps, "RD phy: " + std::to_string(rrftag) + " arch(UNUSED)");
        }

        std::string sim_rs1Valid = valid1 ? "(valid)" : "(false)";
        std::string sim_rs1Sel   = lookupSrcASel(src1_sel);
        writer.addSlotVal(rps, "RS1" + sim_rs1Valid +
                               " /Data: " + std::to_string(src1) +
                               " /Sel: " + sim_rs1Sel);

        std::string sim_rs2Valid = valid2 ? "(valid)" : "(false)";
        std::string sim_rs2Sel   = lookupSrcBSel(src2_sel);
        writer.addSlotVal(rps, "RS2" + sim_rs2Valid +
                               " /Data: " + std::to_string(src2) +
                               " /Sel: " + sim_rs2Sel);
    }

    void SimState::RSV_BRANCH_ENTRY::printDetailedSlot(SlotWriterBase& writer,
                                                     REC_PIP_STAGE rps,
                                                     const COMMIT_STATE& commitState){
        ///// master call
        RSV_BASE_ENTRY::printDetailedSlot(writer, rps, commitState);

        std::string result0 = "IMMBR: " + cvtNum2HexStr(imm_br) + "/ PADDR: " + cvtNum2HexStr(praddr);
        writer.addSlotVal(rps, result0);
        writer.addSlotVal(rps, "opcode: " + cvtNum2HexStr(opcode));
    }

    void SimState::RSV_MUL_ENTRY::printDetailedSlot(SlotWriterBase& writer,
                                                     REC_PIP_STAGE rps,
                                                     const COMMIT_STATE& commitState){
        ////// master call
        RSV_BASE_ENTRY::printDetailedSlot(writer, rps, commitState);
        std::string result0 = "sign1: " + std::to_string(src1_signed) +
                              " /sign2: " + std::to_string(src2_signed) +
                              " /selHi: " + std::to_string(sel_lohi);
        writer.addSlotVal(rps, result0);
    }

    /**
     *  exec slot
     *
     */
    void SimState::EXEC_ALU_STATE::printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState){
        writer.addSlotVal(RPS_EXECUTE, "----> ALU Stage ");
        if (!writeSlotIfRunning(RPS_EXECUTE, st, writer)){return;}
        entry.printDetailedSlot(writer, RPS_EXECUTE, commitState);
    }

    void SimState::EXEC_MUL_STATE::printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState){
        writer.addSlotVal(RPS_EXECUTE, "----> MUL Stage ");
        if (!writeSlotIfRunning(RPS_EXECUTE, st, writer)){return;}
        entry.printDetailedSlot(writer, RPS_EXECUTE, commitState);
    }

    void SimState::EXEC_BRANCH_STATE::printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState, BC bcState){
        writer.addSlotVal(RPS_EXECUTE, "----> BRANCH Stage ");
        if (!writeSlotIfRunning(RPS_EXECUTE, st, writer)){return;}

        if (bcState.misPred){
            writer.addSlotVal(RPS_EXECUTE, "MISPRED");
        }
        if (bcState.succPred){
            writer.addSlotVal(RPS_EXECUTE, "SUCCPRED");
        }

        entry.printDetailedSlot(writer, RPS_EXECUTE, commitState);
    }

    void SimState::EXEC_LDST_STATE::printSlot(SlotWriterBase& writer, COMMIT_STATE& commitState){
        writer.addSlotVal(RPS_EXECUTE, "----> LDST Stage ");
        if (writeSlotIfRunning(RPS_EXECUTE, st1, writer)){
            entry.printDetailedSlot(writer, RPS_EXECUTE, commitState);
            writer.addSlotVal(RPS_EXECUTE, "effAddr " + cvtNum2HexStr(effAddr));
        }


        writer.addSlotVal(RPS_EXECUTE, "-----> EXEC Stage 2");
        if (writeSlotIfRunning(RPS_EXECUTE, st2, writer)){
            writer.addSlotVal(RPS_EXECUTE, "rdUse:" + std::to_string(rdUse) +
                                           " /spec:" + std::to_string(spec) +
                                           " /specTag:" + cvtNum2BinStr(specTag));
            writer.addSlotVal(RPS_EXECUTE, "stbHit:" + std::to_string(stBufHit) +
                                           "stbData:" + cvtNum2HexStr(stBufData));
            writer.addSlotVal(RPS_EXECUTE, "loadData: " + std::to_string(loadData));
        }
    }

    void SimState::COMMIT_ENTRY::printSlot(SlotWriterBase& writer){

        //////// basic string for rob entry
        std::string entryStr = "E:" + std::to_string(idx);
        entryStr += "/fin:" + std::to_string(wbFin);

        entryStr += "/rd:";
        if (rdUse){
            entryStr += std::to_string(rdIdx);
        }else{
            entryStr += "-";
        }

        if (storeBit){
            entryStr += "/Store";
        }
        writer.addSlotVal(RPS_COMMIT, entryStr);
    }


    void SimState::COMMIT_STATE::printSlot(SlotWriterBase& writer){
        writer.addSlotVal(RPS_COMMIT, "comPtr:" + std::to_string(comPtr));
        writer.addSlotVal(RPS_COMMIT, "com1Status:" + std::to_string(com1Status));
        writer.addSlotVal(RPS_COMMIT, "com2Status:" + std::to_string(com2Status));
        if (com1Status){
            comEntries[comPtr % RRF_NUM].printSlot(writer);
        }
        if (com2Status){
            comEntries[(comPtr + 1) % RRF_NUM].printSlot(writer);
        }

        if (isPrevCycleDp1){
            writer.addSlotVal(RPS_COMMIT, "-----------");
            writer.addSlotVal(RPS_COMMIT, "dispatched");
            comEntries[dpPointer % RRF_NUM].printSlot(writer);
            if (isPrevCycleDp2){
                comEntries[(dpPointer + 1) % RRF_NUM].printSlot(writer);
            }

        }

    }

    ////// store buffer stage

    void SimState::STORE_BUF_ENTRY::printSlot(SlotWriterBase& writer){

        if (!busy){return;}

        std::string result0;
        result0 += "[" + std::to_string(idx) + " ";
        result0 += "cpt:" + std::to_string(complete);
        result0 += "sp:" + std::to_string(spec) + " ";
        result0 += "sptg:" + cvtNum2BinStr(specTag) + " ";
        writer.addSlotVal(RPS_STBUF, result0);

        std::string result1;
        result1 += "addr:" + cvtNum2HexStr(mem_addr) + " ";
        result1 += "data:" + cvtNum2HexStr(mem_data);
        writer.addSlotVal(RPS_STBUF, result1);
    }

    void SimState::STORE_BUF_STATE::printSlot(SlotWriterBase& writer){
        writer.addSlotVal(RPS_STBUF, "finPtr: " + std::to_string(finPtr));
        writer.addSlotVal(RPS_STBUF, "comPtr: " + std::to_string(comPtr));
        writer.addSlotVal(RPS_STBUF, "retPtr: " + std::to_string(retPtr));
        for (int idx = 0; idx < STBUF_ENT_NUM; idx++){
            entries[idx].printSlot(writer);
        }
        writer.addSlotVal(RPS_STBUF, "nb1 " + std::to_string(nb1));
        writer.addSlotVal(RPS_STBUF, "ne1 " + std::to_string(ne1));
        writer.addSlotVal(RPS_STBUF, "nb0 " + std::to_string(nb0));
        writer.addSlotVal(RPS_STBUF, "fullNext " + std::to_string(fullNext));
        writer.addSlotVal(RPS_STBUF, "emptyNext " + std::to_string(emptyNext));
    }

    /**
     * register
     ***/

    void SimState::MPFT_STATE::printSlot(SlotWriterBase& writer){
        for (int spIdx = 0; spIdx < SPECTAG_LEN; spIdx++){
            std::string specTagStr = "v: " + std::to_string(valids[spIdx]) + " sp: ";
            for (int idx = (SPECTAG_LEN - 1); idx >= 0; idx--){
                specTagStr += std::to_string(fixTable[spIdx][idx]);
            }
            writer.addSlotVal(RPS_MPFT, specTagStr);
        }
    }

    void SimState::TAGGEN_STATE::printSlot(SlotWriterBase& writer){
        writer.addSlotVal(RPS_DECODE, "tag gen");
        writer.addSlotVal(RPS_DECODE, "branch depth" + std::to_string(brdepth));
        writer.addSlotVal(RPS_DECODE, "tagReg " + cvtNum2BinStr(tagReg));
    }

    void SimState::ARF_STATE::printSlot(SlotWriterBase& writer, BC bcPrev){

        if (bcPrev.misPred){
            writer.addSlotVal(RPS_ARF, "CHANGE FROM MISPRED");
        }else if (bcPrev.succPred){
            writer.addSlotVal(RPS_ARF, "CHANGE FROM SUCCPRED");
        }

        for (int tableIdx = 0; tableIdx < (SPECTAG_LEN+1); tableIdx++){
            writer.addSlotVal(RPS_ARF, "TABLEIDX " + std::to_string(tableIdx));
            std::string reNameVal;
            for (int rrfIdx = 0; rrfIdx < REG_NUM; rrfIdx++){
                if (busy[tableIdx][rrfIdx]){
                    reNameVal += std::to_string(rename[tableIdx][rrfIdx]) + "|";
                }else{
                    reNameVal += "-|";
                }

                if ((rrfIdx % 8) == 7){
                    writer.addSlotVal(RPS_ARF, reNameVal);
                    reNameVal.clear();
                }
            }
        }

    }

    void SimState::RRF_STATE::printSlot(SlotWriterBase& writer){
        writer.addSlotVal(RPS_RRF, "freeNum " + std::to_string(freenum));
        writer.addSlotVal(RPS_RRF, "reqPtr"   + std::to_string(reqPtr));
        writer.addSlotVal(RPS_RRF, "nextCycle" + std::to_string(nextRrfCycle));
        // for (int rrfIdx = 0; rrfIdx < REG_NUM; rrfIdx++){
        //     writer.addSlotVal(RPS_RRF, "RRFIDX " + std::to_string(rrfIdx) +
        //                                 " /V: " + std::to_string(busy[rrfIdx]) +
        //                                 " /DATA:" + cvtNum2HexStr(data[rrfIdx]));
        // }
    }

    void SimState::printSlotWindow(SlotWriterBase& writer){
        fetch.printSlot(writer);
        decode.printSlot(writer);
        dispatch.printSlot(writer);

        /////// alu 1
        writer.addSlotVal(RPS_RSV, "---> ALU Stage 1");
        for (int i = 0; i < ALU_ENT_NUM; i++){
            rsvAlu1[i].printSlot(writer, RPS_RSV);
        }
        /////// alu 2
        writer.addSlotVal(RPS_RSV, "---> ALU Stage 2");
        for (int i = 0; i < ALU_ENT_NUM; i++){
            rsvAlu2[i].printSlot(writer, RPS_RSV);
        }
        writer.addSlotVal(RPS_RSV, "---> MUL Stage");
        for (int i = 0; i < MUL_ENT_NUM; i++){
            rsvMul[i].printSlot(writer, RPS_RSV);
        }
        writer.addSlotVal(RPS_RSV, "---> BR Stage");
        for (int i = 0; i < BRANCH_ENT_NUM; i++){
            rsvBranch[i].printSlot(writer, RPS_RSV);
        }
        writer.addSlotVal(RPS_RSV, "---> LDST Stage");
        for (int i = 0; i < LDST_ENT_NUM; i++){
            rsvLdSt[i].printSlot(writer, RPS_RSV);
        }

        writeIssue("alu1", writer, st_issue_alu1  , idx_issue_alu1, true);
        writeIssue("alu2", writer, st_issue_alu2  , idx_issue_alu2, true);
        writeIssue("mul", writer, st_issue_mul   , idx_issue_mul  , true);
        writeIssue("branch", writer, st_issue_branch, idx_issue_branch, false);
        writeIssue("ldst", writer, st_issue_ldst  , idx_issue_ldst    , false);


        /////// execute unit

        exec_alu1.printSlot(writer, rob);
        exec_alu2.printSlot(writer, rob);
        exec_mul.printSlot(writer, rob);
        exec_branch.printSlot(writer, rob, bcState);
        exec_ldst.printSlot(writer, rob);

        rob.printSlot(writer);
        stbuf.printSlot(writer);

        /////// register architecture

        mpft.printSlot  (writer);
        tagGen.printSlot(writer);
        arf.printSlot   (writer, bcPrev);
        rrf.printSlot   (writer);
    }

}
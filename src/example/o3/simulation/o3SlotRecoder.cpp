//
// Created by tanawin on 14/10/25.
//

#include "o3SlotRecoder.h"

#include "proberGrp.h"

namespace kathryn::o3{



    void O3SlotRecorder::recordSlot(){

        /////// write tag mgmt slot and reg arch first
        writeMpftSlot();
        writeArfSlot();
        writeRrfSlot();
        /////// write frontend
        writeFetchSlot();
        writeDecodeSlot();
        writeDispatchSlot();

        /////// write backend
        writeRsvAluSlot();
        writeRsvBranchSlot();

        writeIssueAluSlot();
        writeIssueBranchSlot();

        writeExecuteAluSlot();
        writeExecuteBranchSlot();

        writeCommitSlot();

    }


    bool O3SlotRecorder::writeSlotIfPipIdle(REC_PIP_STAGE stageIdx,
                                                  PipSimProbe* pipProbe){
        if (pipProbe->isWaiting()){
            _slotWriter->addSlotVal(stageIdx, "IDLE");
            return true;
        }
        return false;
    }

    bool O3SlotRecorder::writeSlotIfZyncStall(REC_PIP_STAGE stageIdx,
                                                  ZyncSimProb* zyncSimProbe){
        if (zyncSimProbe->isWaiting()){
            _slotWriter->addSlotVal(stageIdx, "STALL");
            return true;
        }
        return false;
    }

    void O3SlotRecorder::writeSlotIfTableChange(
        REC_PIP_STAGE stageIdx,
        std::vector<SlotSimInfo64> changeRows,
        int rowLimToPrintEntireRow){ //// amount of row in changing if exceed, we will print only changing field
        bool notExceedRowLim = (changeRows.size() <= rowLimToPrintEntireRow);

        for (SlotSimInfo64& changeRow : changeRows){
            _slotWriter->addSlotVal(stageIdx, "-----> row idx: " + std::to_string(changeRow.rowIdx));
            for (int colIdx = 0; colIdx < changeRow.fieldSimInfos.size(); colIdx++){
                FieldSimInfo64& fieldInfo = changeRow.fieldSimInfos[colIdx];
                if (notExceedRowLim || fieldInfo.prevValue != fieldInfo.curValue){
                    std::string fieldStr = fieldInfo.name + ": " +
                                           std::to_string(fieldInfo.prevValue) + "->" +
                                           std::to_string(fieldInfo.curValue);
                    _slotWriter->addSlotVal(stageIdx, fieldStr);
                }
            }
        }

    }


    void O3SlotRecorder::writeMpftSlot(){
        TableSimProbe& tbProbe = dataStructProbGrp.mpft;
        std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        writeSlotIfTableChange(RPS_MPFT, rowChange, 256);
        _slotWriter->addSlotVal(RPS_MPFT, "----------");
    }

    void O3SlotRecorder::writeArfSlot(){
        _slotWriter->addSlotVal(RPS_ARF, "----busy table---");
        TableSimProbe& tbProbe = dataStructProbGrp.arfBusy;
        std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        writeSlotIfTableChange(RPS_ARF, rowChange, 0);
        _slotWriter->addSlotVal(RPS_ARF, "----------");
        _slotWriter->addSlotVal(RPS_ARF, "----renamed table---");
        TableSimProbe& tbProbe2 = dataStructProbGrp.arfRename;
        std::vector<SlotSimInfo64> rowChange2 = tbProbe2.detectRowChange();
        writeSlotIfTableChange(RPS_ARF, rowChange2, 0);
        _slotWriter->addSlotVal(RPS_ARF, "----------");
    }

    void O3SlotRecorder::writeRrfSlot(){
        TableSimProbe& tbProbe = dataStructProbGrp.rrf;
        std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        writeSlotIfTableChange(RPS_RRF, rowChange, 256);
    }

    void O3SlotRecorder::writeFetchSlot(){
            ////////// write pipe status
            bool idle = writeSlotIfPipIdle(RPS_FETCH, &pipProbGrp.fetch);
            if (idle) {return;}
            writeSlotIfZyncStall(RPS_FETCH, &zyncProbGrp.fetch);
            ////////// write pipe detail
            FetchMod& fetMod = _core->pFetch;

            ///// get the pc
            _slotWriter->addSlotVal(RPS_FETCH, "PC");
            _slotWriter->addSlotVal(RPS_FETCH, cvtNum2HexStr(ull(fetMod.curPc)));
    }

    void O3SlotRecorder::writeDecodeSlot(){
        ////////// write pipe status
        bool idle = writeSlotIfPipIdle(RPS_DECODE, &pipProbGrp.decode);
        if (idle) {return;}
        writeSlotIfZyncStall(RPS_DECODE, &zyncProbGrp.decode);
        ////////// write pipe detail
        FetchStage&  fetch_stage = _ps->ft;


        ull sim_invalid1   = ull(fetch_stage.raw(invalid1));
        ull sim_instr1     = ull(fetch_stage.raw(inst1));
        ull sim_invalid2   = ull(fetch_stage.raw(invalid2));
        ull sim_instr2     = ull(fetch_stage.raw(inst2));
        ull sim_pc         = ull(fetch_stage.raw(pc));
        ull sim_npc        = ull(fetch_stage.raw(npc));

        _slotWriter->addSlotVal(RPS_DECODE, "PC" + cvtNum2HexStr(ull(sim_pc)));
        _slotWriter->addSlotVal(RPS_DECODE, "NPC" + cvtNum2HexStr(ull(sim_npc)));

        for (int i = 1; i <= 2; i++){
            ull sim_invalid = (i == 1) ? sim_invalid1 : sim_invalid2;
            ull sim_instr = (i == 1) ? sim_instr1 : sim_instr2;

            if (sim_invalid){
                _slotWriter->addSlotVal(RPS_DECODE, "notValid");
            }else{
                std::string decStr = "valid OP: ";
                decStr += translateOpcode(sim_instr);
                _slotWriter->addSlotVal(RPS_DECODE, decStr);
            }
        }
    }

    void O3SlotRecorder::writeDispatchSlot(){
        ////////// write pipe status
        bool idle = writeSlotIfPipIdle(RPS_DISPATCH, &pipProbGrp.dispatch);
        if (idle) {return;}
        writeSlotIfZyncStall(RPS_DISPATCH, &zyncProbGrp.dispatch);
        ////////// write pipe detail
        DecodeStage& decode_stage = _ps->dc;

        for (int i = 1; i <= 2; i++){
            RegSlot& targetRegSlot = (i == 1) ? decode_stage.dcd1: decode_stage.dcd2;

            ull sim_invalid = ull(targetRegSlot(invalid));
            ull sim_immType = ull(targetRegSlot(imm_type));
            ull sim_aluOp   = ull(targetRegSlot(aluOp));
            ull sim_rsEnt   = ull(targetRegSlot(rsEnt));
            ull sim_isBranch = ull(targetRegSlot(isBranch));
            ull sim_pred_addr = ull(targetRegSlot(pred_addr));
            ull sim_spec = ull(targetRegSlot(spec));
            ull sim_specTag = ull(targetRegSlot(specTag));
            ull sim_illLegal = ull(targetRegSlot(illLegal));
            ull sim_rdIdx = ull(targetRegSlot(rdIdx));
            ull sim_rdUse = ull(targetRegSlot(rdUse));
            ull sim_rsIdx_1 = ull(targetRegSlot(rsIdx_1));
            ull sim_rsSel_1 = ull(targetRegSlot(rsSel_1));
            ull sim_rsUse_1 = ull(targetRegSlot(rsUse_1));
            ull sim_rsIdx_2 = ull(targetRegSlot(rsIdx_2));
            ull sim_rsSel_2 = ull(targetRegSlot(rsSel_2));
            ull sim_rsUse_2 = ull(targetRegSlot(rsUse_2));

            if (sim_invalid){
                _slotWriter->addSlotVal(RPS_DISPATCH, "notValid");
            }
            else{
                std::map<ull, std::string> immTypeMap = {
                    {0b00, "IMM_I"},
                    {0b01, "IMM_S"},
                    {0b10, "IMM_U"},
                    {0b11, "IMM_J"}
                };
                std::string immTypeStr = (immTypeMap.find(sim_immType) != immTypeMap.end()) ? immTypeMap[sim_immType] : "UNKNOWN";
                _slotWriter->addSlotVal(RPS_DISPATCH, "RS" + std::to_string(sim_rsEnt));
                _slotWriter->addSlotVal(RPS_DISPATCH, "IMM_" + immTypeStr);
                _slotWriter->addSlotVal(RPS_DISPATCH, "ALU" + std::to_string(sim_aluOp));

                if (sim_isBranch){
                    _slotWriter->addSlotVal(RPS_DISPATCH, "isBr: " + std::to_string(sim_isBranch) +
                                                          "/isSp: " + std::to_string(sim_spec) +
                                                          "/spTag: " + cvtNum2BinStr(sim_specTag));
                    _slotWriter->addSlotVal(RPS_DISPATCH, "nextPc_" + cvtNum2HexStr(sim_pred_addr));
                }

                std::string rdUsage = sim_rdUse ? "(USE)" : "(UNUSED)";
                _slotWriter->addSlotVal(RPS_DISPATCH, "RD: "+ rdUsage + " /IDX: " +  std::to_string(sim_rdIdx));

                std::map<ull, std::string> srcASelMap = {
                    {0, "RS1"},
                    {1, "PC"},
                    {2, "ZERO"}
                };

                std::map<ull, std::string> srcBSelMap = {
                    {0, "RS2"},
                    {1, "IMM"},
                    {2, "FOUR"},
                    {3, "ZERO"}
                };
                
                std::string r1Usage = sim_rsUse_1 ? "(USE)" : "(UNUSED)";
                std::string selStr = (srcASelMap.find(sim_rsSel_1) != srcASelMap.end())
                                         ? srcASelMap[sim_rsSel_1]
                                         : "UNKNOWN";
                _slotWriter->addSlotVal(RPS_DISPATCH, "RS1: " + r1Usage + " /IDX:" + std::to_string(sim_rsIdx_1) +
                                        " /SEL:" + selStr);

                std::string r2Usage = sim_rsUse_2 ? "(USE)" : "(UNUSED)";
                std::string selStr2 = (srcBSelMap.find(sim_rsSel_2) != srcBSelMap.end())
                                         ? srcBSelMap[sim_rsSel_2]
                                         : "UNKNOWN";
                _slotWriter->addSlotVal(RPS_DISPATCH, "RS2: " + r2Usage + " /IDX:" + std::to_string(sim_rsIdx_2) +
                                        " /SEL:" + selStr2);
            }
        }
    }


    void O3SlotRecorder::writeRsvAluSlot(){
        /////// write for alu rsv
        _slotWriter->addSlotVal(RPS_RSV, "ALU RSV");
        TableSimProbe& tbProbe = dataStructProbGrp.rsvAlu;
        std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        writeSlotIfTableChange(RPS_RSV, rowChange, 2);
        _slotWriter->addSlotVal(RPS_RSV, "----------");
    }

    void O3SlotRecorder::writeRsvBranchSlot(){
        /////// write for branch rsv
        _slotWriter->addSlotVal(RPS_RSV, "BRANCH RSV");
        TableSimProbe& tbProbe = dataStructProbGrp.rsvbranch;
        std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        writeSlotIfTableChange(RPS_RSV, rowChange, 2);
        _slotWriter->addSlotVal(RPS_RSV, "----------");
    }

    void O3SlotRecorder::writeIssueAluSlot(){
        /////// write for alu issue
        _slotWriter->addSlotVal(RPS_ISSUE, "ALU ISSUE");
        bool isStall = writeSlotIfZyncStall(RPS_ISSUE, &zyncProbGrp.issueAlu);
        if (!isStall){
            _slotWriter->addSlotVal(RPS_ISSUE, "issue Enty: " + cvtNum2BinStr(ull(_core->aluRsv.checkIdx)));
        }
        _slotWriter->addSlotVal(RPS_ISSUE, "----------");
        
    }

    void O3SlotRecorder::writeIssueBranchSlot(){
        /////// write for branch issue
        _slotWriter->addSlotVal(RPS_ISSUE, "BRANCH ISSUE");
        bool isStall = writeSlotIfZyncStall(RPS_ISSUE, &zyncProbGrp.issueBranch);
        if (!isStall){
            _slotWriter->addSlotVal(RPS_ISSUE, "issue Enty: " + std::to_string(ull(_core->branchRsv.checkIdx)));
        }
        _slotWriter->addSlotVal(RPS_ISSUE, "----------");
    }


    void O3SlotRecorder::writeExecuteBasic(RegSlot& src){
        ull sim_pc = ull(src(pc));
        ull sim_imm = ull(src(imm));
        ull sim_rrftag = ull(src(rrftag));
        ull sim_rdUse = ull(src(rdUse));
        ull sim_aluOp = ull(src(aluOp));
        ull sim_spec = ull(src(spec));
        ull sim_specTag = ull(src(specTag));
        ull sim_phyIdx_1 = ull(src(phyIdx_1));
        ull sim_rsSel_1 = ull(src(rsSel_1));
        ull sim_rsValid_1 = ull(src(rsValid_1));
        ull sim_phyIdx_2 = ull(src(phyIdx_2));
        ull sim_rsSel_2 = ull(src(rsSel_2));
        ull sim_rsValid_2 = ull(src(rsValid_2));
        
        _slotWriter->addSlotVal(RPS_EXECUTE, "PC: " + cvtNum2HexStr(sim_pc));
        _slotWriter->addSlotVal(RPS_EXECUTE, "IMM: " + cvtNum2HexStr(sim_imm));
        _slotWriter->addSlotVal(RPS_EXECUTE, "ALU Op: " + std::to_string(sim_aluOp) +
                                             "/Spec: " + std::to_string(sim_spec) +
                                             "/SpecTag: " + cvtNum2BinStr(sim_specTag));
        std::string sim_isRdUsed = sim_rdUse ? "(USE)" : "(UNUSED)";
        _slotWriter->addSlotVal(RPS_EXECUTE, "RD" + sim_isRdUsed + " rd:" + std::to_string(sim_rrftag));


        std::map<ull, std::string> srcASelMap = {
            {0, "RS1"},
            {1, "PC"},
            {2, "ZERO"}
        };

        std::map<ull, std::string> srcBSelMap = {
            {0, "RS2"},
            {1, "IMM"},
            {2, "FOUR"},
            {3, "ZERO"}
        };
        /////////// rs1
        std::string sim_rs1Valid = sim_rsValid_1 ? "(valid)" : "(false)";
        std::string selStr = (srcASelMap.find(sim_rsSel_1) != srcASelMap.end())
                                         ? srcASelMap[sim_rsSel_1]
                                         : "UNKNOWN";
        _slotWriter->addSlotVal(RPS_EXECUTE, "RS1" + sim_rs1Valid +
                                             " /Data: " + std::to_string(sim_phyIdx_1) +
                                             " /Sel: " + selStr);
        /////////// rs2
        std::string sim_rs2Valid = sim_rsValid_2 ? "(valid)" : "(false)";
        std::string selStr2 = (srcBSelMap.find(sim_rsSel_2) != srcBSelMap.end())
                                         ? srcBSelMap[sim_rsSel_2]
                                         : "UNKNOWN";
        _slotWriter->addSlotVal(RPS_EXECUTE, "RS2" + sim_rs2Valid +
                                " /Data: " + std::to_string(sim_phyIdx_2) +
                                " /Sel: " + selStr2);

    }



    void O3SlotRecorder::writeExecuteAluSlot(){

        _slotWriter->addSlotVal(RPS_EXECUTE, "ALU EXEC");
        bool aluIdle = writeSlotIfPipIdle(RPS_EXECUTE, &pipProbGrp.execAlu);
        if (aluIdle){return;}

        writeExecuteBasic(_core->aluRsv.execSrc);

    }

    void O3SlotRecorder::writeExecuteBranchSlot(){
        _slotWriter->addSlotVal(RPS_EXECUTE, "BRANCH EXEC");
        bool branchIdle = writeSlotIfPipIdle(RPS_EXECUTE, &pipProbGrp.execBranch);
        if (branchIdle){return;}

        _slotWriter->addSlotVal(RPS_EXECUTE, "OP: " + translateOpcode(ull(_core->branchRsv.execSrc(opcode))));
        _slotWriter->addSlotVal(RPS_EXECUTE, "---");
        writeExecuteBasic(_core->branchRsv.execSrc);
    }

    void O3SlotRecorder::writeCommitSlot(){

        int amtCommit = static_cast<int>(ull(_core->prob.com1Status) && ull(_core->prob.com2Status));

        _slotWriter->addSlotVal(RPS_COMMIT, "---- changing");
        TableSimProbe& tbProbe = dataStructProbGrp.commit;
        std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        writeSlotIfTableChange(RPS_COMMIT, rowChange, 256);
        _slotWriter->addSlotVal(RPS_COMMIT, "----- committing");


        _slotWriter->addSlotVal(RPS_COMMIT, "cmPtr: " + cvtNum2HexStr(ull(_core->prob.comPtr)));
        _slotWriter->addSlotVal(RPS_COMMIT, "cmAmt: " + std::to_string(amtCommit));
        if (amtCommit == 0){ return;}

        for (int i = 1; i <= 2; i++){
            WireSlot& targetRegSlot = (i == 1) ? _core->prob.com1Entry : _core->prob.com2Entry;

            ull sim_wbFin    = ull(targetRegSlot(wbFin));
            ull sim_isBranch = ull(targetRegSlot(isBranch));
            ull sim_rdUse    = ull(targetRegSlot(rdUse));
            ull sim_rdIdx    = ull(targetRegSlot(rdIdx));

            std::string entryStr = "E" + std::to_string(i) + ": ";
            entryStr += "fin=" + std::to_string(sim_wbFin);
            entryStr += "/isBr:" + std::to_string(sim_isBranch);
            entryStr += "/rdUse:" + std::to_string(sim_rdUse);
            entryStr += "/rdIdx:" + std::to_string(sim_rdIdx);

            _slotWriter->addSlotVal(RPS_COMMIT, entryStr);


        }

    }

    std::string O3SlotRecorder::translateOpcode(ull rawInstr){
        std::map<ull, std::string> decMap = {
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

        ull opMaskBit = (1 << 7) - 1;
        ull op = rawInstr & opMaskBit;
        return (decMap.find(op) != decMap.end()) ? decMap[op] : "UNKNOWN";
    }
}

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
        writeRsvAluSlot(1, _core->rsvs.alu1);
        writeRsvAluSlot(2, _core->rsvs.alu2);
        writeRsvMulSlot();
        writeRsvBranchSlot();
        writeRsvLoadSlot();

        /////// write backend

        ///////// rsv issue
        writeIssueAluSlot(1, _core->rsvs.alu1, zyncProbGrp.issueAlu1);
        writeIssueAluSlot(2, _core->rsvs.alu2, zyncProbGrp.issueAlu2);
        writeIssueMulSlot();
        writeIssueBranchSlot();
        writeIssueLdStSlot();

        ///////// exec issue
        writeExecuteAluSlot(1, _core->rsvs.alu1, pipProbGrp.execAlu1);
        writeExecuteAluSlot(2, _core->rsvs.alu2, pipProbGrp.execAlu2);
        writeExecuteMulSlot();
        auto [thisCycleMis, thisCycleSuc] =
        writeExecuteBranchSlot();
        writeExecuteLdStSlot();

        ///////// write commit stage
        writeCommitSlot();

        //////// iterate the cycle
        _slotWriter->iterateCycle();

        ////// update MisPred Status
        isLastCycleMisPred = thisCycleMis;
        isLastCycleSucPred = thisCycleSuc;
        ////// update dispatch cycle
        lastDispatchPtr  = ull(_core->regArch.rrf.getReqPtr());
        isLastCycleDisp1 = ull(_core->pDisp.dbg_isDisp1);
        isLastCycleDisp2 = ull(_core->pDisp.dbg_isDisp2);
        ////// apply change on slot recorder
        dataStructProbGrp.applyCycleChange();
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
        int rowLimToPrintEntireRow) const{ //// amount of row in changing if exceed, we will print only changing field
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

        _slotWriter->addSlotVal(RPS_MPFT, "brdepth: " + std::to_string(ull(_core->tagMgmt.tagGen.brdepth)));
        _slotWriter->addSlotVal(RPS_MPFT, "tagReg: " + cvtNum2BinStr(ull(_core->tagMgmt.tagGen.tagreg)));

        Table& mpftTable = _core->tagMgmt.mpft._table;
        for (int rowIdx = 0; rowIdx < mpftTable.getNumRow(); rowIdx++){
            RegSlot& entry = mpftTable(rowIdx);
            ull sim_valid  = ull(entry(mpft_valid));
            ull sim_fixTag = ull(entry(mpft_fixTag));
            _slotWriter->addSlotVal(RPS_MPFT,
                "vl: " + std::to_string(sim_valid) + " "
                "-> " + cvtNum2BinStr(sim_fixTag)  + " "
                "idx " + std::to_string(rowIdx));
        }
        _slotWriter->addSlotVal(RPS_MPFT, "----------");
    }

    std::vector<std::string> O3SlotRecorder::getArfSlotVal(RegSlot& busyEntry, RegSlot& renameEntry){
        std::vector<std::string> result;
        const int rowNum = 4;
        const int colNum = REG_NUM/rowNum;

        for (int row = 0; row < rowNum; row++){
            std::string rowStr;
            for (int col = 0; col < colNum; col++){
                int idx = row * colNum + col;
                ull sim_busy = ull(busyEntry(idx));
                ull sim_rename = ull(renameEntry(idx));
                rowStr += sim_busy ? std::to_string(sim_rename) : "-";
                if (col < colNum - 1){
                    rowStr += "|";
                }
            }
            result.push_back(rowStr);
        }
        return result;
    }

    void O3SlotRecorder::writeArfSlot(){
        ////// TODO the data change may be cumbersome, we have to find the way to fix this situation
        if (isLastCycleMisPred){
            _slotWriter->addSlotVal(RPS_ARF, "CHANGE FROM MISPRED");
        }else if (isLastCycleSucPred){
            _slotWriter->addSlotVal(RPS_ARF, "CHANGE FROM SUCCPRED");
        }

        /////// for each spectag
        for (int tableIdx = 0; tableIdx < SPECTAG_LEN; tableIdx++){
            std::vector<std::string> arfTable =
                getArfSlotVal(_core->regArch.arf.busy(tableIdx),
                              _core->regArch.arf.rename(tableIdx));

            _slotWriter->addSlotVal(RPS_ARF, "SPECTAG: " + std::to_string(tableIdx));
            for (const auto & rowIdx : arfTable){
                _slotWriter->addSlotVal(RPS_ARF, rowIdx);
            }
        }
        /////// for master
        std::vector<std::string> arfTableMaster =
            getArfSlotVal(_core->regArch.arf.busyMaster,
                          _core->regArch.arf.renameMaster);
        _slotWriter->addSlotVal(RPS_ARF, "MASTER");
        for (const auto & rowIdx : arfTableMaster){
            _slotWriter->addSlotVal(RPS_ARF, rowIdx);
        }

    }

    void O3SlotRecorder::writeRrfSlot() const{

        ull sim_reqPtr    = ull(_core->regArch.rrf.reqPtr);
        ull sim_comPtr    = ull(_core->prob.comPtr);
        ull sim_freeNum   = ull(_core->regArch.rrf.freenum);
        ull sim_nextCycle = ull(_core->regArch.rrf.nextRrfCycle);

        std::string turnStr =  (sim_comPtr <= sim_reqPtr) ? "COM->REQ" : "REQ->COM (LB)";
        _slotWriter->addSlotVal(RPS_RRF, turnStr);
        _slotWriter->addSlotVal(RPS_RRF, "REQ PTR: " + std::to_string(sim_reqPtr));
        _slotWriter->addSlotVal(RPS_RRF, "COM PTR: " + std::to_string(sim_comPtr));
        _slotWriter->addSlotVal(RPS_RRF, "FREE SIZE" + std::to_string(sim_freeNum));
        _slotWriter->addSlotVal(RPS_RRF, "NEXT CY: " + std::to_string(sim_nextCycle));

        _slotWriter->addSlotVal(RPS_RRF, "--------");
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
        _slotWriter->addSlotVal(RPS_DECODE, "gennable "+ std::to_string(ull(_core->pDec.dbg_isGenable)));
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

        RegSlot& decShared = _ps->dc.dcdShared;
        ull sim_shared_pc = ull(decShared(pc));
        ull sim_shared_desEqSrc1 = ull(decShared(desEqSrc1));
        ull sim_shared_desEqSrc2 = ull(decShared(desEqSrc2));
        _slotWriter->addSlotVal(RPS_DISPATCH, "aluRsvAble: " + std::to_string(ull(_core->pDisp.dbg_isAluRsvAllocatable)));
        _slotWriter->addSlotVal(RPS_DISPATCH, "brRsvAble: " + std::to_string(ull(_core->pDisp.dbg_isBranchRsvAllocatable)));
        _slotWriter->addSlotVal(RPS_DISPATCH, "isRenam: " + std::to_string(ull(_core->pDisp.dbg_isRenamable)));
        _slotWriter->addSlotVal(RPS_DISPATCH, "PC: " + cvtNum2HexStr(sim_shared_pc));
        std::string internalDep = str("S1EqDes: ") + (sim_shared_desEqSrc1? "1 " : "0 ") +
                                  str("S2EqDes: ") + (sim_shared_desEqSrc2? "1 " : "0 ");

        _slotWriter->addSlotVal(RPS_DISPATCH, internalDep);

        for (int i = 1; i <= 2; i++){
            RegSlot& targetRegSlot = (i == 1) ? decode_stage.dcd1: decode_stage.dcd2;

            ull sim_invalid   = ull(targetRegSlot(invalid));
            ull sim_immType   = ull(targetRegSlot(imm_type));
            ull sim_aluOp     = ull(targetRegSlot(aluOp));
            ull sim_rsEnt     = ull(targetRegSlot(rsEnt));
            ull sim_isBranch  = ull(targetRegSlot(isBranch));
            ull sim_pred_addr = ull(targetRegSlot(pred_addr));
            ull sim_spec      = ull(targetRegSlot(spec));
            ull sim_specTag   = ull(targetRegSlot(specTag));
            ull sim_illLegal  = ull(targetRegSlot(illLegal));
            ull sim_rdIdx     = ull(targetRegSlot(rdIdx));
            ull sim_rdUse     = ull(targetRegSlot(rdUse));
            ull sim_rsIdx_1   = ull(targetRegSlot(rsIdx_1));
            ull sim_rsSel_1   = ull(targetRegSlot(rsSel_1));
            ull sim_rsUse_1   = ull(targetRegSlot(rsUse_1));
            ull sim_rsIdx_2   = ull(targetRegSlot(rsIdx_2));
            ull sim_rsSel_2   = ull(targetRegSlot(rsSel_2));
            ull sim_rsUse_2   = ull(targetRegSlot(rsUse_2));

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
                std::map<ull, std::string> rsvTypeMap = {
                    {RS_ENT_ALU    , "ALU"},
                    {RS_ENT_BRANCH , "BRANCH"},
                    {RS_ENT_JAL    , "JAL"},
                    {RS_ENT_JALR   , "JALR"},
                    {RS_ENT_MUL    , "MUL"},
                    {RS_ENT_DIV    , "DIV"},
                    {RS_ENT_LDST   , "LDST"}
                };
                std::string immTypeStr = (immTypeMap.find(sim_immType) != immTypeMap.end()) ? immTypeMap[sim_immType] : "UNKNOWN";
                std::string rsTypeStr = (rsvTypeMap.find(sim_rsEnt) != rsvTypeMap.end()) ? rsvTypeMap[sim_rsEnt] : "UNKNOWN";
                _slotWriter->addSlotVal(RPS_DISPATCH, "RS: " + rsTypeStr);
                _slotWriter->addSlotVal(RPS_DISPATCH, immTypeStr);
                _slotWriter->addSlotVal(RPS_DISPATCH, "ALU: " + translateAluOp(sim_aluOp));


                _slotWriter->addSlotVal(RPS_DISPATCH, "isBr: " + std::to_string(sim_isBranch) +
                                                      "/isSp: " + std::to_string(sim_spec) +
                                                      "/spTag: " + cvtNum2BinStr(sim_specTag));
                _slotWriter->addSlotVal(RPS_DISPATCH, "nextPc_" + cvtNum2HexStr(sim_pred_addr));

                std::string rdUsage = sim_rdUse ? "(USE)" : "(UNUSED)";
                _slotWriter->addSlotVal(RPS_DISPATCH, "RD: "+ rdUsage + " /ArchIdx: " +  std::to_string(sim_rdIdx));

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

    /**
     *
     * RSV writing section
     *
     */

    std::pair<bool, std::vector<std::string>> O3SlotRecorder::writeRsvSlot(RegSlot& entry){

        ///// entry identifier
        ull sim_busy      = ull(entry(busy));

        ull sim_pc        = ull(entry(pc));
        ull sim_rrftag    = ull(entry(rrftag));
        ull sim_rdUse     = ull(entry(rdUse));
        ull sim_spec      = ull(entry(spec));
        ull sim_specTag   = ull(entry(specTag));
        ull sim_phyIdx_1  = ull(entry(phyIdx_1));
        ull sim_rsSel_1   = ull(entry(rsSel_1));
        ull sim_rsValid_1 = ull(entry(rsValid_1));
        ull sim_phyIdx_2  = ull(entry(phyIdx_2));
        ull sim_rsSel_2   = ull(entry(rsSel_2));
        ull sim_rsValid_2 = ull(entry(rsValid_2));

        if (!sim_busy){
            return{false, {}};
        }

        std::string result0;

        if (entry.isThereField(sortBit)){
            ull sim_sortBit = ull(entry(sortBit));
            result0 += "sb:" + std::string(sim_sortBit ? "1" : "0") + " ";
        }

        result0 += "pc:" + cvtNum2HexStr(sim_pc) + " ";
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

        std::string result1;
        result1 += "spec: " + std::to_string(sim_spec) + " spt:" + cvtNum2BinStr(sim_specTag);

        return {true, {result0, result1}};
    }


    void O3SlotRecorder::writeRsvBasicSlot(Table& table){

        for (int rowIdx = 0; rowIdx < table.getNumRow(); rowIdx++){
            RegSlot& entry = table(rowIdx);
            bool isUsed = false;
            std::vector<std::string> results;
            std::tie(isUsed, results) = writeRsvSlot(entry);
            if (isUsed){
                bool isFirst = true;
                for (std::string& result : results){
                    std::string prefix = isFirst ? (std::to_string(rowIdx) + "] ") : "";
                    _slotWriter->addSlotVal(RPS_RSV, prefix + result);
                    isFirst = false;
                }

            }
        }

    }


    void O3SlotRecorder::writeRsvAluSlot(int idx, ORsv& orsv){
        /////// write for alu rsv
        _slotWriter->addSlotVal(RPS_RSV, "ALU RSV " + std::to_string(idx));
        writeRsvBasicSlot(orsv._table);
        _slotWriter->addSlotVal(RPS_RSV, "----------");
    }

    void O3SlotRecorder::writeRsvMulSlot(){
        ORsv& mulRsv = _core->rsvs.mul;
        _slotWriter->addSlotVal(RPS_RSV, "MUL RSV ");
        writeRsvBasicSlot(mulRsv._table);
        _slotWriter->addSlotVal(RPS_RSV, "----------");
    }


    void O3SlotRecorder::writeRsvBranchSlot(){
        /////// write for branch rsv
        IRsv& branchIRsv = _core->rsvs.br;
        _slotWriter->addSlotVal(RPS_RSV, "BRANCH RSV");
        _slotWriter->addSlotVal(RPS_RSV, "allocPtr : " + std::to_string(ull(branchIRsv.allocPtr)));
        writeRsvBasicSlot(branchIRsv._table);
        _slotWriter->addSlotVal(RPS_RSV, "----------");
    }

    void O3SlotRecorder::writeRsvLoadSlot(){
        IRsv& ldstIRsv = _core->rsvs.ls;
        _slotWriter->addSlotVal(RPS_RSV, "LDST RSV");
        _slotWriter->addSlotVal(RPS_RSV, "allocPtr : " + std::to_string(ull(ldstIRsv.allocPtr)));
        writeRsvBasicSlot(ldstIRsv._table);
        _slotWriter->addSlotVal(RPS_RSV, "----------");
    }


    /**
     *
     * RSV writing section
     *
     */

    void O3SlotRecorder::writeIssueAluSlot(int idx, ORsv& orsv, ZyncSimProb& zIssueProbe){
        /////// write for alu issue
        _slotWriter->addSlotVal(RPS_ISSUE, "ALU ISSUE " + std::to_string(idx));
        bool isStall = writeSlotIfZyncStall(RPS_ISSUE, &zIssueProbe);
        if (!isStall){
            _slotWriter->addSlotVal(RPS_ISSUE, "issue Enty: " + cvtNum2BinStr(ull(orsv.checkIdx)));
        }
        _slotWriter->addSlotVal(RPS_ISSUE, "slotReady: " + std::to_string(ull(orsv.dbg_isSlotReady)));
        _slotWriter->addSlotVal(RPS_ISSUE, "----------");
        
    }

    void O3SlotRecorder::writeIssueMulSlot(){
        ORsv& mulRsv = _core->rsvs.mul;
        _slotWriter->addSlotVal(RPS_ISSUE, "MUL ISSUE");
        bool isStall = writeSlotIfZyncStall(RPS_ISSUE, &zyncProbGrp.issueMul);
        if (!isStall){
            _slotWriter->addSlotVal(RPS_ISSUE, "issue Enty: " + std::to_string(ull(mulRsv.checkIdx)));
        }
    }

    void O3SlotRecorder::writeIssueBranchSlot(){
        /////// write for branch issue
        _slotWriter->addSlotVal(RPS_ISSUE, "BRANCH ISSUE");
        bool isStall = writeSlotIfZyncStall(RPS_ISSUE, &zyncProbGrp.issueBranch);
        if (!isStall){
            IRsv& branchIRsv = _core->rsvs.br;
            _slotWriter->addSlotVal(RPS_ISSUE, "issue Enty: " + std::to_string(ull(branchIRsv.checkIdx)));
        }
        _slotWriter->addSlotVal(RPS_ISSUE, "----------");
    }

    void O3SlotRecorder::writeIssueLdStSlot(){
        /////// write for branch issue
        _slotWriter->addSlotVal(RPS_ISSUE, "LDST ISSUE");
        bool isStall = writeSlotIfZyncStall(RPS_ISSUE, &zyncProbGrp.issueLdSt);
        if (!isStall){
            IRsv& ldStIRsv = _core->rsvs.ls;
            _slotWriter->addSlotVal(RPS_ISSUE, "issue Enty: " + std::to_string(ull(ldStIRsv.checkIdx)));
        }
        _slotWriter->addSlotVal(RPS_ISSUE, "----------");
    }


    void O3SlotRecorder::writeExecuteBasic(RegSlot& src){
        ull sim_pc        = ull(src(pc));
        ull sim_imm       = 777;
        if (src.isThereField(imm)){
            sim_imm = ull(src(imm));
        }else{
            sim_imm       = ull(src(imm_br));
        }
        ull sim_rrftag    = ull(src(rrftag));
        ull sim_rdUse     = ull(src(rdUse));
        ull sim_aluOp     = ull(src(aluOp));
        ull sim_spec      = ull(src(spec));
        ull sim_specTag   = ull(src(specTag));
        ull sim_phyIdx_1  = ull(src(phyIdx_1));
        ull sim_rsSel_1   = ull(src(rsSel_1));
        ull sim_rsValid_1 = ull(src(rsValid_1));
        ull sim_phyIdx_2  = ull(src(phyIdx_2));
        ull sim_rsSel_2   = ull(src(rsSel_2));
        ull sim_rsValid_2 = ull(src(rsValid_2));

        std::map<ull, std::string> aluOpMap = {
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
        
        _slotWriter->addSlotVal(RPS_EXECUTE, "PC: " + cvtNum2HexStr(sim_pc));
        _slotWriter->addSlotVal(RPS_EXECUTE, "IMM: " + cvtNum2HexStr(sim_imm));
        _slotWriter->addSlotVal(RPS_EXECUTE, "ALU Op: " + translateAluOp(sim_aluOp) +
                                             "/Spec: " + std::to_string(sim_spec) +
                                             "/SpecTag: " + cvtNum2BinStr(sim_specTag));
        std::string sim_isRdUsed = sim_rdUse ? "(USE)" : "(UNUSED)";
        if (sim_rdUse){
            RegSlot&  targetRegSlot = _core->prob._table(static_cast<int>(sim_rrftag));
            ull sim_rdIdx    = ull(targetRegSlot(rdIdx));

            _slotWriter->addSlotVal(RPS_EXECUTE, "RD phy: " + std::to_string(sim_rrftag) + " arch: " + std::to_string(sim_rdIdx));
        }else{
            _slotWriter->addSlotVal(RPS_EXECUTE, "RD phy: " + std::to_string(sim_rrftag) + " arch(UNUSED)");
        }



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



    void O3SlotRecorder::writeExecuteAluSlot(int idx, ORsv& orsv, PipSimProbe& pExecProbe){

        _slotWriter->addSlotVal(RPS_EXECUTE, "ALU EXEC " + std::to_string(idx));
        bool aluIdle = writeSlotIfPipIdle(RPS_EXECUTE, &pExecProbe);
        if (aluIdle){return;}

        writeExecuteBasic(orsv.execSrc);
        _slotWriter->addSlotVal(RPS_EXECUTE, "----------");
    }

    void O3SlotRecorder::writeExecuteMulSlot(){
        ORsv& mulRsv = _core->rsvs.mul;
        _slotWriter->addSlotVal(RPS_EXECUTE, "MUL EXEC");
        bool mulIdle = writeSlotIfPipIdle(RPS_EXECUTE, &pipProbGrp.execMul);
        if (mulIdle){return;}
        writeExecuteBasic(mulRsv.execSrc);
        _slotWriter->addSlotVal(RPS_EXECUTE, "----------");
    }

    std::pair<bool, bool> O3SlotRecorder::writeExecuteBranchSlot(){
        _slotWriter->addSlotVal(RPS_EXECUTE, "BRANCH EXEC");
        bool branchIdle = writeSlotIfPipIdle(RPS_EXECUTE, &pipProbGrp.execBranch);
        if (branchIdle){return {false, false};}

        bool isThisCycleMisPred = false;
        bool isThisCycleSucc = false;

        _slotWriter->addSlotVal(RPS_EXECUTE, "---BRANCH STATUS");
        if (ull(_core->tagMgmt.bc.mis)){
            _slotWriter->addSlotVal(RPS_EXECUTE, "MISPREDICTED");
            isThisCycleMisPred = true;
        }
        if (ull(_core->tagMgmt.bc.suc)){
            _slotWriter->addSlotVal(RPS_EXECUTE, "SUC PREDICTED");
            isLastCycleMisPred = true;

        }
        IRsv& branchIRsv = _core->rsvs.br;
        _slotWriter->addSlotVal(RPS_EXECUTE, "OP: " + translateOpcode(ull(branchIRsv.execSrc(opcode))));
        _slotWriter->addSlotVal(RPS_EXECUTE, "---");
        writeExecuteBasic(branchIRsv.execSrc);

        return {isThisCycleMisPred, isThisCycleSucc};
    }

    void O3SlotRecorder::writeExecuteLdStSlot(){
        IRsv& ldstIRsv = _core->rsvs.ls;
        ////////////////// load store 1
        _slotWriter->addSlotVal(RPS_EXECUTE, "LDST EXEC 1");
        bool ldStIdle = writeSlotIfPipIdle(RPS_EXECUTE, &pipProbGrp.execLdSt);
        if (!ldStIdle){
            writeExecuteBasic(ldstIRsv.execSrc);
        }
        _slotWriter->addSlotVal(RPS_EXECUTE, "----------");

        ////////////////// load store 2
        _slotWriter->addSlotVal(RPS_EXECUTE, "LDST EXEC 2");
        bool ldStIdle2 = writeSlotIfPipIdle(RPS_EXECUTE, &pipProbGrp.execLdSt2);
        if (ldStIdle2){return;}
        writeExecuteBasic(_core->pm.ldSt.lsRes);
    }

    std::vector<std::string> O3SlotRecorder::writeRobSlot(ull robIdx){

        std::vector<std::string> result;

        RegSlot&  targetRegSlot = _core->prob._table(static_cast<int>(robIdx));

        ull sim_wbFin    = ull(targetRegSlot(wbFin));
        ull sim_isBranch = ull(targetRegSlot(isBranch));
        ull sim_rdUse    = ull(targetRegSlot(rdUse));
        ull sim_rdIdx    = ull(targetRegSlot(rdIdx));

        ull sim_storeBit = ull(targetRegSlot(storeBit));

        ull sim_pc       = ull(targetRegSlot(pc));
        ull sim_jumpAddr = ull(targetRegSlot(jumpAddr));
        ull sim_jumpCond = ull(targetRegSlot(jumpCond));


        //////// basic string for rob entry
        std::string entryStr = "E:" + std::to_string(robIdx);
        entryStr += "/fin:" + std::to_string(sim_wbFin);

        entryStr += "/rd:";
        if (sim_rdUse){
            entryStr += std::to_string(sim_rdIdx);
        }else{
            entryStr += "-";
        }

        if (sim_storeBit){
            entryStr += "/Store";
        }
        result.push_back(entryStr);

        //////// branch info add in case it is branch
        if (sim_isBranch){
            result.push_back("/br: c  "    + std::to_string(sim_jumpCond));
            result.push_back("/br: pc "  + cvtNum2HexStr(sim_pc));
            result.push_back("/br: to "  + cvtNum2HexStr(sim_jumpAddr));
        }
        return result;

    }


    void O3SlotRecorder::writeCommitSlot(){


        _slotWriter->addSlotVal(RPS_COMMIT, "----- dispatched (prevCycle)");
        int amtDisp = static_cast<int>(isLastCycleDisp1 + isLastCycleDisp2);
        int robSize = 1 << _core->regArch.rrf.getReqPtr().getOperableSlice().getSize();
        _slotWriter->addSlotVal(RPS_COMMIT, "dispPtr: " + std::to_string(ull(_core->regArch.rrf.reqPtr)));
        _slotWriter->addSlotVal(RPS_COMMIT, "dispAmt: " + std::to_string(ull(amtDisp)));
        for (int i = 0; i < amtDisp; i++){
            _slotWriter->addSlotVals(RPS_COMMIT, writeRobSlot((lastDispatchPtr + i)%robSize));
        }


        _slotWriter->addSlotVal(RPS_COMMIT, "----- committing");
        int amtCommit = static_cast<int>(ull(_core->prob.com1Status) + ull(_core->prob.com2Status));
        _slotWriter->addSlotVal(RPS_COMMIT, "cmPtr: " + std::to_string(ull(_core->prob.comPtr)));
        _slotWriter->addSlotVal(RPS_COMMIT, "cmAmt: " + std::to_string(amtCommit));

        for (int i = 0; i < amtCommit; i++){
            _slotWriter->addSlotVals(RPS_COMMIT, writeRobSlot((ull(_core->prob.comPtr) + i) % robSize));
        }

        // _slotWriter->addSlotVal(RPS_COMMIT, "----- changing");
        // TableSimProbe& tbProbe = dataStructProbGrp.commit;
        // std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        // writeSlotIfTableChange(RPS_COMMIT, rowChange, 256);

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

    std::string O3SlotRecorder::translateAluOp(ull aluOpIdx){
        std::map<ull, std::string> aluOpMap = {
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
        return (aluOpMap.find(aluOpIdx) != aluOpMap.end()) ? aluOpMap[aluOpIdx] : "UNKNOWN";
    }

}

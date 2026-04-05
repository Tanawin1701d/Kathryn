//
// Created by tanawin on 24/12/25.
//

#include "simStateKride.h"


namespace kathryn::o3{

    pipStat SimStateKride::generatePipState(PipSimProbe* pipProbe,
                             ZyncSimProb* zyncSimProbe){

        assert((pipProbe != nullptr) || (zyncSimProbe != nullptr));

        if (pipProbe != nullptr &&
            pipProbe->isWaiting()){ return PS_IDLE;  }

        if (zyncSimProbe == nullptr) { return PS_RUNNING; }
        if (zyncSimProbe->isWaiting()){ return PS_STALL; }
        return PS_RUNNING;
    }

    void SimStateKride::assignEleIfThere(ull& ele, RegSlot& val, const std::string& eleName){
        if (val.isThereField(eleName)){
            ele = ull(val(eleName));
        }
    }

    void SimStateKride::assignRSV_Entry(RSV_BASE_ENTRY& entry, RegSlot& regSlot){

        assignEleIfThere(entry.busy   , regSlot, busy);
        assignEleIfThere(entry.sortbit, regSlot, sortBit);
        assignEleIfThere(entry.pc     , regSlot, pc);
        assignEleIfThere(entry.imm    , regSlot, imm);
        assignEleIfThere(entry.rrftag , regSlot, rrftag);
        assignEleIfThere(entry.dstval , regSlot, rdUse);
        assignEleIfThere(entry.aluOp , regSlot, aluOp);
        assignEleIfThere(entry.specBit, regSlot, spec);
        assignEleIfThere(entry.spectag, regSlot, specTag);

        assignEleIfThere(entry.src1    , regSlot, phyIdx_1);
        assignEleIfThere(entry.src1_sel, regSlot, rsSel_1);
        assignEleIfThere(entry.valid1  , regSlot, rsValid_1);
        assignEleIfThere(entry.src2    , regSlot, phyIdx_2);
        assignEleIfThere(entry.src2_sel, regSlot, rsSel_2);
        assignEleIfThere(entry.valid2  , regSlot, rsValid_2);

    }

    void SimStateKride::assignRSV_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& regSlot){

        assignRSV_Entry(entry, regSlot);
        assignEleIfThere(entry.immBr  , regSlot, immBr);
        assignEleIfThere(entry.praddr  , regSlot, predAddr);
        assignEleIfThere(entry.opcode  , regSlot, opcode);

    }

    void SimStateKride::assignRSV_Mul(RSV_MUL_ENTRY& entry, RegSlot& regSlot){
        assignRSV_Entry(entry, regSlot);
        assignEleIfThere(entry.src1_signed, regSlot, mdReqInSigned1);
        assignEleIfThere(entry.src2_signed, regSlot, mdReqInSigned2);
        assignEleIfThere(entry.selLohi   , regSlot, mdReqOutSel);

        entry.pc      = 0; //// mul and ldst discard it
        entry.imm     = 0;
        entry.aluOp  = 0; //////  mul and ldst discard it
        entry.src1_sel = 0, entry.src2_sel = 0; ///// ldst discards it




    }


    void SimStateKride::assignEXEC_Entry (RSV_BASE_ENTRY& entry  , RegSlot& regSlot){
        assignRSV_Entry(entry, regSlot);
        entry.busy    = 0;
        entry.sortbit = 0;
        entry.valid1  = 0;
        entry.valid2  = 0;
    }
    void SimStateKride::assignEXEC_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& regSlot){
        assignRSV_Branch(entry, regSlot);
        entry.busy     = 0;
        entry.sortbit  = 0;
        entry.imm      = 0;
        entry.src1_sel = 0;
        entry.valid1   = 0;
        entry.src2_sel = 0;
        entry.valid2   = 0;

    }

    void SimStateKride::assignEXEC_Mul(RSV_MUL_ENTRY& entry, RegSlot& regSlot){
        assignRSV_Mul(entry, regSlot);
        entry.busy     = 0;
        entry.sortbit  = 0;
        entry.imm      = 0;
        entry.aluOp   = 0;
        entry.src1_sel = 0;
        entry.valid1   = 0;
        entry.src2_sel = 0;
        entry.valid2   = 0;
    }
    void SimStateKride::assignEXEC_LDST(RSV_BASE_ENTRY& entry, RegSlot& regSlot){
        assignRSV_Entry(entry, regSlot);
        entry.busy     = 0;
        entry.sortbit  = 0;
        entry.aluOp   = 0;
        entry.src1_sel = 0;
        entry.valid1   = 0;
        entry.src2_sel = 0;
        entry.valid2   = 0;
    }


    void SimStateKride::assignARF_Table(int tableIdx,RegSlot& busySlot, RegSlot& renameSlot){
        for (int regIdx = 0; regIdx < REG_NUM; regIdx++){
            arf.busy  [tableIdx][regIdx] = (ull(busySlot  (regIdx)) != 0);
            arf.rename[tableIdx][regIdx] = ull(renameSlot(regIdx));
        }
    }






    void SimStateKride::recruitValue(){

        //////////  bcState
        bcState.misPred  = ull(_core.tagMgmt.bc.mis) != 0;
        bcState.succPred = ull(_core.tagMgmt.bc.suc) != 0;
        bcPrev.misPred  = isLastCycleMisPred;
        bcPrev.succPred = isLastCycleSucc;

        //// fetch
        fetch.st = generatePipState(&pipProbGrp.fetch, &zyncProbGrp.fetch);
        fetch.pc = ull(_core.pFetch.curPc);

        //// decode
        FetchStage&  fetchStage = _core.pm.ft;
        decode.st       = generatePipState(&pipProbGrp.decode, &zyncProbGrp.decode);
        decode.inst1    = ull(fetchStage.raw(inst1)   );
        decode.invalid2 = ull(fetchStage.raw(invalid2));
        decode.inst2    = ull(fetchStage.raw(inst2)   );
        decode.pc       = ull(fetchStage.raw(pc)      );
        decode.npc      = ull(fetchStage.raw(npc)     );
        decode.isGenable= (ull(_core.pDec.dbg_isGenable) != 0);

        //// dispatch
        RegSlot& decShared = _core.pm.dc.dcdShared;

        dispatch.st = generatePipState(&pipProbGrp.dispatch, &zyncProbGrp.dispatch);
        dispatch.pc        = ull(decShared(pc));
        dispatch.desEqSrc1 = ull(decShared(desEqSrc1));
        dispatch.desEqSrc2 = ull(decShared(desEqSrc2));

        DecodeStage& decodeStage = _core.pm.dc;
        for (int i = 1; i <= 2; i++){
            RegSlot& targetRegSlot = (i == 1) ? decodeStage.dcd1 : decodeStage.dcd2;
            DispInstr& dispState   = (i == 1) ? dispatch.dp1      : dispatch.dp2;

            dispState.invalid     = ull(targetRegSlot(invalid));
            dispState.immType    = ull(targetRegSlot(immType));
            dispState.aluOp       = ull(targetRegSlot(aluOp));
            dispState.rsEnt       = ull(targetRegSlot(rsEnt));
            dispState.isBranch    = ull(targetRegSlot(isBranch));
            dispState.predAddr   = ull(targetRegSlot(predAddr));
            dispState.spec        = ull(targetRegSlot(spec));
            dispState.specTag     = ull(targetRegSlot(specTag));
            dispState.rdIdx       = ull(targetRegSlot(rdIdx));
            dispState.rdUse       = ull(targetRegSlot(rdUse));
            dispState.rsIdx_1     = ull(targetRegSlot(rsIdx_1));
            dispState.rsSel_1     = ull(targetRegSlot(rsSel_1));
            dispState.rsUse_1     = ull(targetRegSlot(rsUse_1));
            dispState.rsIdx_2     = ull(targetRegSlot(rsIdx_2));
            dispState.rsSel_2     = ull(targetRegSlot(rsSel_2));
            dispState.rsUse_2     = ull(targetRegSlot(rsUse_2));
        }

        dispatch.isAluRsvAllocatable    = ull(_core.pDisp.dbg_isAluRsvAllocatable) != 0;
        dispatch.isBranchRsvAllocatable = ull(_core.pDisp.dbg_isBranchRsvAllocatable) != 0;
        dispatch.isRenamable            = ull(_core.pDisp.dbg_isRenamable) != 0;


        //// reservation station
        for(int idx = 0; idx < ALU_ENT_NUM; idx++){
            assignRSV_Entry(rsvAlu1[idx], _core.rsvs.alu1._table(idx));
            assignRSV_Entry(rsvAlu2[idx], _core.rsvs.alu2._table(idx));
        }
        stIssueAlu1  = generatePipState(nullptr, &zyncProbGrp.issueAlu1);
        idxIssueAlu1 = ull(_core.rsvs.alu1.checkIdx);
        stIssueAlu2  = generatePipState(nullptr, &zyncProbGrp.issueAlu2);
        idxIssueAlu2 = ull(_core.rsvs.alu2.checkIdx);

        for (int idx = 0; idx < MUL_ENT_NUM; idx++){
            assignRSV_Mul(rsvMul[idx], _core.rsvs.mul._table(idx));
        }
        stIssueMul  = generatePipState(nullptr, &zyncProbGrp.issueMul);
        idxIssueMul = ull(_core.rsvs.mul.checkIdx);

        for (int idx = 0; idx < BRANCH_ENT_NUM; idx++){
            assignRSV_Branch(rsvBranch[idx], _core.rsvs.br._table(idx));
        }
        stIssueBranch  = generatePipState(nullptr, &zyncProbGrp.issueBranch);
        idxIssueBranch = ull(_core.rsvs.br.checkIdx);

        for (int idx = 0; idx < LDST_ENT_NUM; idx++){
            assignRSV_Entry(rsvLdSt[idx], _core.rsvs.ls._table(idx));
        }
        stIssueLdst  = generatePipState(nullptr, &zyncProbGrp.issueLdSt);
        idxIssueLdst = ull(_core.rsvs.ls.checkIdx);

        //////////////////////
        //// execute /////////
        //////////////////////

        execAlu1.st = generatePipState(&pipProbGrp.execAlu1, nullptr);
        assignEXEC_Entry(execAlu1.entry, _core.rsvs.alu1.execSrc);
        execAlu2.st = generatePipState(&pipProbGrp.execAlu2, nullptr);
        assignEXEC_Entry(execAlu2.entry, _core.rsvs.alu2.execSrc);

        execMul.st = generatePipState(&pipProbGrp.execMul, nullptr);
        assignEXEC_Mul(execMul.entry, _core.rsvs.mul.execSrc);

        execBranch.st = generatePipState(&pipProbGrp.execBranch, nullptr);
        assignEXEC_Branch(execBranch.entry, _core.pExBra.src);

        execLdst.st1 = generatePipState(&pipProbGrp.execLdSt, &zyncProbGrp.loadStore2);
        execLdst.st2 = generatePipState(&pipProbGrp.execLdSt2, nullptr);
        assignEXEC_LDST(execLdst.entry, _core.rsvs.ls.execSrc);
        execLdst.effAddr = ull(_core.pExLdSt.dbg_effAddr);

        execLdst.rrftag    =   ull(_core.pExLdSt.lsRes(rrftag));
        execLdst.rdUse     =   ull(_core.pExLdSt.lsRes(rdUse));
        execLdst.spec      =   ull(_core.pExLdSt.lsRes(spec));
        execLdst.specTag   =   ull(_core.pExLdSt.lsRes(specTag));
        execLdst.stBufData =   ull(_core.pExLdSt.lsRes(stBufData));
        execLdst.stBufHit  =   ull(_core.pExLdSt.lsRes(stBufHit));
        execLdst.loadData = ull(_core.pExLdSt.lss.dmemRdata);

        //// commit
        rob.comPtr = ull(_core.prob.comPtr);
        rob.com1Status = ull(_core.prob.com1Status) != 0;
        rob.com2Status = ull(_core.prob.com2Status) != 0;
        for(int rrfIdx = 0; rrfIdx < RRF_NUM; rrfIdx++){
            rob.comEntries[rrfIdx].wbFin    = ull(_core.prob._table(rrfIdx)(wbFin).v()   );
            rob.comEntries[rrfIdx].storeBit = ull(_core.prob._table(rrfIdx)(storeBit).v());
            rob.comEntries[rrfIdx].rdUse    = ull(_core.prob._table(rrfIdx)(rdUse).v()   );
            rob.comEntries[rrfIdx].rdIdx    = ull(_core.prob._table(rrfIdx)(rdIdx).v()   );
        }
        rob.isPrevCycleDp1 = isLastCycleDisp1;
        rob.isPrevCycleDp2 = isLastCycleDisp2;
        rob.dpPointer      = ull(lastDispatchPtr);


        //// store buffer
        stbuf.finPtr        = ull(_core.storeBuf.finPtr);
        stbuf.comPtr        = ull(_core.storeBuf.comPtr);
        stbuf.retPtr        = ull(_core.storeBuf.retPtr);
        stbuf.nb1           = ull(_core.storeBuf.nb1.sIdx);
        stbuf.ne1           = ull(_core.storeBuf.ne1.sIdx);
        stbuf.nb0           = ull(_core.storeBuf.nb0.sIdx);
        stbuf.fullNext      = ull(_core.storeBuf.fullNext) != 0;
        stbuf.emptyNext     = ull(_core.storeBuf.emptyNext) != 0;
        for (int storeBufIdx = 0; storeBufIdx < STBUF_ENT_NUM; storeBufIdx++){
            stbuf.entries[storeBufIdx].busy     = ull(_core.storeBuf._table(storeBufIdx)(busy).v());
            stbuf.entries[storeBufIdx].complete = ull(_core.storeBuf._table(storeBufIdx)(complete).v());
            stbuf.entries[storeBufIdx].spec     = ull(_core.storeBuf._table(storeBufIdx)(spec).v());
            stbuf.entries[storeBufIdx].specTag  = ull(_core.storeBuf._table(storeBufIdx)(specTag).v());
            stbuf.entries[storeBufIdx].memAddr = ull(_core.storeBuf._table(storeBufIdx)(memAddr).v());
            stbuf.entries[storeBufIdx].memData = ull(_core.storeBuf.daytas.at(storeBufIdx));
        }

        /////// mpft

        Mpft& hwMpft = _core.tagMgmt.mpft;
        for(int spRowIdx = 0; spRowIdx < SPECTAG_LEN; spRowIdx++){
            mpft.valids[spRowIdx] = ull(hwMpft.isUsed(spRowIdx)) != 0;
            for (int spColIdx = 0; spColIdx < SPECTAG_LEN; spColIdx++){
                ull rowValue = ull(hwMpft._table(spRowIdx)(mpft_fixTag));
                ull colValue = ull(rowValue >> spColIdx);
                mpft.fixTable[spRowIdx][spColIdx] = ull((colValue & 1) != 0);
            }
        }

        /////// tag generator
        tagGen.brdepth = ull(_core.tagMgmt.tagGen.brdepth);
        tagGen.tagReg  = ull(_core.tagMgmt.tagGen.tagreg);
        /////// ARF
        for(int tableIdx = 0; tableIdx < SPECTAG_LEN; tableIdx++){
            assignARF_Table(tableIdx, _core.regArch.arf.busy(tableIdx),
                                      _core.regArch.arf.rename(tableIdx));
        }
        assignARF_Table(SPECTAG_LEN, _core.regArch.arf.busyMaster,
                                     _core.regArch.arf.renameMaster);

        /////// rrf
        Table& rrfTable = _core.regArch.rrf.table;
        for (int idx = 0; idx < RRF_NUM; idx++){
            rrf.busy[idx] = ull(rrfTable(idx)(rrfValid).v());
            rrf.data[idx] = ull(rrfTable(idx)(rrfData ).v());
        }
        rrf.freenum       = ull(_core.regArch.rrf.freenum);
        rrf.reqPtr        = ull(_core.regArch.rrf.reqPtr);
        rrf.nextRrfCycle  = ull(_core.regArch.rrf.nextRrfCycle);
    }

    void SimStateKride::recruitNextCycle(){
        isLastCycleMisPred = ull(_core.tagMgmt.bc.mis) != 0;
        isLastCycleSucc    = ull(_core.tagMgmt.bc.suc) != 0;
        ///////// state recorder for next cycle
        isLastCycleDisp1   = ull(_core.pDisp.dbg_isDisp1);
        isLastCycleDisp2   = ull(_core.pDisp.dbg_isDisp2);
        lastDispatchPtr    = ull(_core.regArch.rrf.getReqPtr());
    }

    void SimStateKride::printSlotWindow(SlotWriterBase& writer){
        ////////// main printing
        SimState::printSlotWindow(writer);
        ///////// rrf change printing
        TableSimProbe& tbProbe = dataStructProbGrp.rrf;
        std::vector<SlotSimInfo64> rowChange = tbProbe.detectRowChange();
        writeSlotIfTableChange(writer, RPS_RRF, rowChange, 256);
        dataStructProbGrp.applyCycleChange();
    }

    void SimStateKride::writeSlotIfTableChange(
        SlotWriterBase& writer,
        REC_PIP_STAGE stageIdx,
        std::vector<SlotSimInfo64> changeRows,
        int rowLimToPrintEntireRow) const{ //// amount of row in changing if exceed, we will print only changing field
        bool notExceedRowLim = (changeRows.size() <= rowLimToPrintEntireRow);

        for (SlotSimInfo64& changeRow : changeRows){
            writer.addSlotVal(stageIdx, "-----> row idx: " + std::toString(changeRow.rowIdx));
            for (int colIdx = 0; colIdx < changeRow.fieldSimInfos.size(); colIdx++){
                FieldSimInfo64& fieldInfo = changeRow.fieldSimInfos[colIdx];
                if (notExceedRowLim || fieldInfo.prevValue != fieldInfo.curValue){
                    std::string fieldStr = fieldInfo.name + ": " +
                                           std::toString(fieldInfo.prevValue) + "->" +
                                           std::toString(fieldInfo.curValue);
                    writer.addSlotVal(stageIdx, fieldStr);
                }
            }
        }

    }



}
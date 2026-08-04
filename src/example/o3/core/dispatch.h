//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H
#define KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

#include "kathryn.h"

#include "immGen.h"
#include "immBrGen.h"

#include "irsv.h"
#include "orsv.h"
#include "rsvs.h"
#include "srcOpr.h"
#include "rob.h"
#include "stageStruct.h"
#include "example/o3/simulation/proberGrp.h"


namespace kathryn::o3{

    struct DpMod: Module{   ///MD DISPATCH
        PipStage& pm;   ///CTRL_HC+DATA_HC DISPATCH
        Rsvs&     rsvs;   ///CTRL_HC+DATA_HC DISPATCH
        RegArch&  regArch;   ///CTRL_HC+DATA_HC DISPATCH
        TagMgmt&  tagMgmt;   ///CTRL_HC DISPATCH
        Rob&      rob;   ///CTRL_HC+DATA_HC DISPATCH

        RegSlot& dcd1     = pm.dc.dcd1;   ///CTRL_HC+DATA_HC DISPATCH
        RegSlot& dcd2     = pm.dc.dcd2;   ///CTRL_HC+DATA_HC DISPATCH
        RegSlot& dcdShare = pm.dc.dcdShared;   ///CTRL_HC+DATA_HC DISPATCH
                                    //// it join the two rsv together
        mWire(aluRsvIdx2_final   , ALU_ENT_SEL + 1);  //// it is one hot index   ///CTRL_HWD DISPATCH
        mWire(mulRsvIdx2_final   , MUL_ENT_SEL);                                 ///CTRL_HWD DISPATCH
        mWire(branchRsvIdx2_final, BRANCH_ENT_SEL); //// it is binary index      ///CTRL_HWD DISPATCH
        mWire(lsRsvIdx2_final    , LDST_ENT_SEL);                                ///CTRL_HWD DISPATCH

        mWire(dbg_isAluRsvAllocatable, 1);      ///DC
        mWire(dbg_isBranchRsvAllocatable, 1);   ///DC
        mWire(dbg_isRenamable, 1);              ///DC
        mWire(dbg_imm1, DATA_LEN);              ///DC
        mWire(dbg_imm2, DATA_LEN);              ///DC

        mWire(dbg_isDisp1, 1); ///DC
        mWire(dbg_isDisp2, 1); ///DC

        DpMod(PipStage& pm    , Rsvs& rsvs,             ///CTRL_HC+DATA_HC DISPATCH
              RegArch& regArch, TagMgmt& tagMgmt,       ///CTRL_HC+DATA_HC DISPATCH
              Rob& rob):                                ///CTRL_HC+DATA_HC DISPATCH
            pm     (pm)     , rsvs   (rsvs),            ///CTRL_HC+DATA_HC DISPATCH
            regArch(regArch), tagMgmt(tagMgmt),         ///CTRL_HC+DATA_HC DISPATCH
            rob    (rob){}                              ///CTRL_HC+DATA_HC DISPATCH

        Operable& isRsvRequired(RegSlot& dcd, int RS_ENT_IDX){     ///CTRL_HC+DATA_HC DISPATCH
            return (dcd(rsEnt) == RS_ENT_IDX) & (~dcd(invalid));   ///CTRL_CL DISPATCH
        }

        ///// check that target rsv is ready to handle them all
        Operable& isAlocatableForRsv(opr& busy1, opr& busy2, int RS_ENT_IDX){   ///CTRL_HC DISPATCH
            return  ((~busy1).uext(2) + (~busy2).uext(2)) >=                    ///CTRL_CL DISPATCH
                (isRsvRequired(pm.dc.dcd1, RS_ENT_IDX).uext(2) +                ///CTRL_CL DISPATCH
                 isRsvRequired(pm.dc.dcd2, RS_ENT_IDX).uext(2));                ///CTRL_CL DISPATCH
        }

        void onSucPred(opr& sucTag){                                 ///CTRL_HC DISPATCH
            dcd1(spec) <<= dcd1(spec) & (dcd1(specTag) != sucTag);   ///CTRL_CL DISPATCH
            dcd2(spec) <<= dcd2(spec) & (dcd2(specTag) != sucTag);   ///CTRL_CL DISPATCH
        }



        WireSlot cvtdecInstrToRsv(RegSlot& dcd, RegSlot& dcdShard, ///CTRL_HC+DATA_HC DISPATCH
                                  opr* desRrf , int decLaneIdx){   ///CTRL_HC DISPATCH
            /////// decLaneIdx start from 0
            /////// create rsv smRsvI for inorder is redundant
            WireSlot des(smRsvO      + smRsvBase +          ///CTRL_HWD+DATA_HWD DISPATCH
                         smRsvBranch + smRsvAlu  +          ///CTRL_HWD+DATA_HWD DISPATCH
                         smRsvMul); /// smRsvBase + smRsvOI ///CTRL_HWD+DATA_HWD DISPATCH
            /////// metadata
            des(busy)    = 1;       ///CTRL_DT DISPATCH
            des(sortBit) = 1;       ///CTRL_DT DISPATCH
            ////////////// base
            if (decLaneIdx == 0){             ///HLH DISPATCH
                des(pc) = dcdShard(pc);       ///DATA_DT DISPATCH
            }else{
                des(pc) = dcdShard(pc) + 4;   ///DATA_CL DISPATCH
            }
            immGen( dcd(inst), dcd(imm_type), des(imm));   ///DATA_HC DISPATCH
            immBrGen(dcd(inst), des(imm_br));              ///DATA_HC DISPATCH

            des(rrftag)  = regArch.rrf.getReqPtr() + decLaneIdx;   ///CTRL_CL DISPATCH
            des(rdUse)   = dcd(rdUse);                             ///CTRL_DT DISPATCH
            des(aluOp)   = dcd(aluOp);                             ///DATA_DT DISPATCH
            des(spec)    = dcd(spec);                              ///CTRL_DT DISPATCH
            des(specTag) = dcd(specTag);                           ///CTRL_DT DISPATCH

            opr* isDesPrevUse1 = nullptr;   ///HLH DISPATCH
            opr* isDesPrevUse2 = nullptr;   ///HLH DISPATCH

            if (desRrf != nullptr){  //// use desRrf as the trigger to check des of previous instr   ///HLH DISPATCH
                isDesPrevUse1 = &dcdShard(desEqSrc1);   ///HLH DISPATCH
                isDesPrevUse2 = &dcdShard(desEqSrc2);   ///HLH DISPATCH

            }

            des(phyIdx_1, rsValid_1) = decodeSrcOpr(dcd, desRrf, isDesPrevUse1,   ///CTRL_HC+DATA_HC DISPATCH
                                1, regArch);                                      ///CTRL_HC+DATA_HC DISPATCH
            des(phyIdx_2, rsValid_2) = decodeSrcOpr(dcd, desRrf, isDesPrevUse2,   ///CTRL_HC+DATA_HC DISPATCH
                                2, regArch);                                      ///CTRL_HC+DATA_HC DISPATCH
            ////////////// branch
            des(opcode)    = dcd(inst)(0, 7);                                     ///DATA_CL DISPATCH
            des(pred_addr) = dcd(pred_addr);                                      ///DATA_DT DISPATCH

            return des;   //////CTRL_HC+DATA_HC DISPATCH
        }

        void flow() override{   ///HLH DISPATCH

            /**
             * RSV CALCULATION
             */

            //////// alu index calculation
            auto[aluRsvBusy , aluRsvIdx ] = rsvs.alu1.buildFreeIndex(nullptr   , &rsvs.alu2);   ///HLH DISPATCH
            auto[aluRsvBusy2, aluRsvIdx2] = rsvs.alu1.buildFreeIndex(&aluRsvIdx, &rsvs.alu2);   ///HLH DISPATCH
            opr& isAluRsvAllocatable = isAlocatableForRsv(aluRsvBusy, aluRsvBusy2, RS_ENT_ALU);   ///CTRL_CL DISPATCH
            aluRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_ALU, aluRsvIdx2, aluRsvIdx);   ///CTRL_CL DISPATCH

            //////// mul index calculation
            auto[mulRsvBusy , mulRsvIdx ] = rsvs.mul.buildFreeIndex(nullptr   );   ///HLH DISPATCH
            auto[mulRsvBusy2, mulRsvIdx2] = rsvs.mul.buildFreeIndex(&mulRsvIdx);   ///HLH DISPATCH
            opr& isMulRsvAllocatable = isAlocatableForRsv(mulRsvBusy, mulRsvBusy2, RS_ENT_MUL);   ///CTRL_CL DISPATCH
            mulRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_MUL, mulRsvIdx2, mulRsvIdx);   ///CTRL_CL DISPATCH

            //////// branch index calculation
            auto[branchRsvBusy , branchRsvIdx ] = rsvs.br.buildFreeIndex(nullptr);   ///HLH DISPATCH
            auto[branchRsvBusy2, branchRsvIdx2] = rsvs.br.buildFreeIndex(&(branchRsvIdx+1));   ///HLH DISPATCH
            opr& isBranchRsvAllocatable = isAlocatableForRsv(branchRsvBusy, branchRsvBusy2, RS_ENT_BRANCH);   ///CTRL_CL DISPATCH
            branchRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_BRANCH, branchRsvIdx2, branchRsvIdx);   ///CTRL_CL DISPATCH

            //////// ls index calculation
            auto[lsRsvBusy , lsRsvIdx ] = rsvs.ls.buildFreeIndex(nullptr);   ///HLH DISPATCH
            auto[lsRsvBusy2, lsRsvIdx2] = rsvs.ls.buildFreeIndex(&(lsRsvIdx+1));   ///HLH DISPATCH
            opr& isLsRsvAllocatable = isAlocatableForRsv(lsRsvBusy, lsRsvBusy2, RS_ENT_LDST);   ///CTRL_CL DISPATCH
            lsRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_LDST, lsRsvIdx2, lsRsvIdx);   ///CTRL_CL DISPATCH

            ///// rename command
            RenameCmd renCmd1{dcd1(rdUse)                 , regArch.rrf.getReqPtr(),   ///CTRL_HC DISPATCH
                              dcd1(rdIdx)                 ,                            ///DATA_HC DISPATCH
                              dcd1(isBranch)              , dcd1(specTag)};            ///CTRL_HC DISPATCH
            RenameCmd renCmd2{dcd2(rdUse)&(~dcd2(invalid)), regArch.rrf.getReqPtr()+1, ///CTRL_HC DISPATCH
                              dcd2(rdIdx)                 ,                            ///DATA_HC DISPATCH
                              dcd2(isBranch)              , dcd2(specTag)};            ///CTRL_HC DISPATCH
            ///// dispatch signal
            opr& isRenamable = regArch.rrf.isRenamable(~dcd2(invalid));          ///CTRL_CL DISPATCH
            opr& isdispatable = isAluRsvAllocatable    & isMulRsvAllocatable &   ///CTRL_CL DISPATCH
                                isBranchRsvAllocatable & isLsRsvAllocatable &    ///CTRL_CL DISPATCH
                                isRenamable;                                     ///CTRL_CL DISPATCH
            ////// pre assign the data to update reservation station
            WireSlot entry1(cvtdecInstrToRsv(dcd1, dcdShare, nullptr        , 0));   ///CTRL_HWD+DATA_HWD DISPATCH
            WireSlot entry2(cvtdecInstrToRsv(dcd2, dcdShare, &entry1(rrftag), 1));   ///CTRL_HWD+DATA_HWD DISPATCH

            ////// dbg zone
            dbg_isAluRsvAllocatable      = isAluRsvAllocatable;    ///DC
            dbg_isBranchRsvAllocatable   = isBranchRsvAllocatable; ///DC
            dbg_isRenamable              = isRenamable;            ///DC
            dbg_imm1                     = entry1(imm);            ///DC
            dbg_imm2                     = entry2(imm);            ///DC

            pip(pm.sync_dp){                               initProbe(pipProbGrp .dispatch);   ///CTRL_HWD+CTRL_CL DISPATCH
                zyncc(pm.sync_rs, isdispatable){ autoSync  initProbe(zyncProbGrp.dispatch);   ///CTRL_HWD+CTRL_CL DISPATCH
                    ////////
                    regArch.rrf.onRename(~dcd2(invalid));    ///CTRL_HC DISPATCH
                    opr& reqPtr = regArch.rrf.getReqPtr();   ///CTRL_DT DISPATCH
                    //////// update arf
                    regArch.arf.onRename(renCmd1, renCmd2);   ///CTRL_HC+DATA_HC DISPATCH
                    ////// dcd 1 supposed to be valid all the time
                    /***
                     * dispatch entry 1
                     */
                    zif (aluRsvIdx.sl(0)){                                     ///CTRL_CL DISPATCH
                        rsvs.alu2.tryWriteEntry(dcd1(rsEnt),                   ///DATA_HC DISPATCH
                                                aluRsvIdx.sl(1, RS_ENT_SEL+1), ///CTRL_HC DISPATCH
                                                entry1);                       ///DATA_HC DISPATCH
                    }zelse{
                        rsvs.alu1.tryWriteEntry(dcd1(rsEnt),   ///DATA_HC DISPATCH
                                                aluRsvIdx.sl(1, RS_ENT_SEL+1), ///CTRL_HC DISPATCH
                                                entry1); ///DATA_HC DISPATCH
                    }
                    rsvs.mul.tryWriteEntry(dcd1(rsEnt), mulRsvIdx   , entry1);   ///CTRL_HC+DATA_HC DISPATCH
                    rsvs.br .tryWriteEntry(dcd1(rsEnt), branchRsvIdx, entry1);   ///CTRL_HC+DATA_HC DISPATCH
                    rsvs.ls .tryWriteEntry(dcd1(rsEnt), lsRsvIdx    , entry1);   ///CTRL_HC+DATA_HC DISPATCH
                    rob.onDispatch(reqPtr, dcd1, dcdShare); //// acknowledge reroder buffer   ///CTRL_HC+DATA_HC DISPATCH
                    dbg_isDisp1 = 1; ///DC
                    /***
                     * dispatch entry 2
                     */
                    zif(~dcd2(invalid)){   ///CTRL_CL DISPATCH
                        zif (aluRsvIdx2_final.sl(0)){   ///CTRL_CL DISPATCH
                            rsvs.alu2.tryWriteEntry(dcd2(rsEnt),   ///DATA_HC DISPATCH
                                                    aluRsvIdx2_final(1, RS_ENT_SEL+1), ///CTRL_HC DISPATCH
                                                    entry2); ///DATA_HC DISPATCH
                        }zelse{
                            rsvs.alu1.tryWriteEntry(dcd2(rsEnt),   ///DATA_HC DISPATCH
                                                    aluRsvIdx2_final.sl(1, RS_ENT_SEL+1), ///CTRL_HC DISPATCH
                                                    entry2); ///DATA_HC DISPATCH
                        }
                        rsvs.mul.tryWriteEntry(dcd2(rsEnt), mulRsvIdx2_final   , entry2);   ///CTRL_HC+DATA_HC DISPATCH
                        rsvs.br .tryWriteEntry(dcd2(rsEnt), branchRsvIdx2_final, entry2);   ///CTRL_HC+DATA_HC DISPATCH
                        rsvs.ls .tryWriteEntry(dcd2(rsEnt), lsRsvIdx2_final    , entry2);   ///CTRL_HC+DATA_HC DISPATCH
                        rob.onDispatch(reqPtr+1, dcd2, dcdShare);   ///CTRL_HC+DATA_HC DISPATCH
                        dbg_isDisp2 = 1; ///DC
                    }
                }
            }
        }
    };
}
#endif //KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

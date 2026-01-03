//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H
#define KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

#include "kathryn.h"

#include "parameter.h"
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

    struct DpMod: Module{
        PipStage& pm;
        Rsvs&     rsvs;
        RegArch&  regArch;
        TagMgmt&  tagMgmt;
        Rob&      rob;

        RegSlot& dcd1     = pm.dc.dcd1;
        RegSlot& dcd2     = pm.dc.dcd2;
        RegSlot& dcdShare = pm.dc.dcdShared;
                                    //// it join the two rsv together
        mWire(aluRsvIdx2_final   , ALU_ENT_SEL + 1);  //// it is one hot index
        mWire(mulRsvIdx2_final   , MUL_ENT_SEL);
        mWire(branchRsvIdx2_final, BRANCH_ENT_SEL); //// it is binary index
        mWire(lsRsvIdx2_final    , LDST_ENT_SEL);

        mWire(dbg_isAluRsvAllocatable, 1);
        mWire(dbg_isBranchRsvAllocatable, 1);
        mWire(dbg_isRenamable, 1);

        mWire(dbg_isDisp1, 1);
        mWire(dbg_isDisp2, 1);

        DpMod(PipStage& pm    , Rsvs& rsvs,
              RegArch& regArch, TagMgmt& tagMgmt,
              Rob& rob):
            pm     (pm),
            rsvs   (rsvs),
            regArch(regArch),
            tagMgmt(tagMgmt),
            rob    (rob){}

        Operable& isRsvRequired(RegSlot& dcd, int RS_ENT_IDX){
            return (dcd(rsEnt) == RS_ENT_IDX) & (~dcd(invalid));
        }

        ///// check that target rsv is ready to handle them all
        Operable& isAlocatableForRsv(opr& busy1, opr& busy2, int RS_ENT_IDX){
            return  ((~busy1).uext(2) + (~busy2).uext(2)) >=
                (isRsvRequired(pm.dc.dcd1, RS_ENT_IDX).uext(2) +
                 isRsvRequired(pm.dc.dcd2, RS_ENT_IDX).uext(2));
        }

        void onSucPred(opr& sucTag){
            dcd1(spec) <<= dcd1(spec) & (dcd1(specTag) != sucTag);
            dcd2(spec) <<= dcd2(spec) & (dcd2(specTag) != sucTag);
        }



        WireSlot cvtdecInstrToRsv(RegSlot& dcd, RegSlot& dcdShard, opr* desRrf , int decLaneIdx){
            /////// decLaneIdx start from 0
            /////// create rsv smRsvI for inorder is redundant
            WireSlot des(smRsvO      + smRsvBase +
                         smRsvBranch + smRsvAlu  +
                         smRsvMul); /// smRsvBase + smRsvOI
            /////// metadata
            des(busy)    = 1;
            des(sortBit) = 1;
            ////////////// base
            if (decLaneIdx == 0){
                des(pc) = dcdShard(pc);
            }else{
                des(pc) = dcdShard(pc) + 4;
            }
            immGen( dcd(inst), dcd(imm_type), des(imm));
            immBrGen(dcd(inst), des(imm_br));

            des(rrftag)  = regArch.rrf.getReqPtr() + decLaneIdx;
            des(rdUse)   = dcd(rdUse);
            des(aluOp)   = dcd(aluOp);
            des(spec)    = dcd(spec);
            des(specTag) = dcd(specTag);

            opr* isDesPrevUse1 = nullptr;
            opr* isDesPrevUse2 = nullptr;

            if (desRrf != nullptr){  //// use desRrf as the trigger to check des of previous instr
                isDesPrevUse1 = &dcdShard(desEqSrc1);
                isDesPrevUse2 = &dcdShard(desEqSrc2);

            }

            des(phyIdx_1, rsValid_1) = decodeSrcOpr(dcd, desRrf, isDesPrevUse1,
                                1, regArch);
            des(phyIdx_2, rsValid_2) = decodeSrcOpr(dcd, desRrf, isDesPrevUse2,
                                2, regArch);
            ////////////// branch
            des(opcode)    = dcd(inst)(0, 7);
            des(pred_addr) = dcd(pred_addr);

            return des;
        }

        void flow() override{

            /**
             * RSV CALCULATION
             */

            //////// alu index calculation
            auto[aluRsvBusy , aluRsvIdx ] = rsvs.alu1.buildFreeIndex(nullptr   , &rsvs.alu2);
            auto[aluRsvBusy2, aluRsvIdx2] = rsvs.alu1.buildFreeIndex(&aluRsvIdx, &rsvs.alu2);
            opr& isAluRsvAllocatable = isAlocatableForRsv(aluRsvBusy, aluRsvBusy2, RS_ENT_ALU);
            aluRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_ALU, aluRsvIdx2, aluRsvIdx);

            //////// mul index calculation
            auto[mulRsvBusy , mulRsvIdx ] = rsvs.mul.buildFreeIndex(nullptr   );
            auto[mulRsvBusy2, mulRsvIdx2] = rsvs.mul.buildFreeIndex(&mulRsvIdx);
            opr& isMulRsvAllocatable = isAlocatableForRsv(mulRsvBusy, mulRsvBusy2, RS_ENT_MUL);
            mulRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_MUL, mulRsvIdx2, mulRsvIdx);

            //////// branch index calculation
            auto[branchRsvBusy , branchRsvIdx ] = rsvs.br.buildFreeIndex(nullptr);
            auto[branchRsvBusy2, branchRsvIdx2] = rsvs.br.buildFreeIndex(&(branchRsvIdx+1));
            opr& isBranchRsvAllocatable = isAlocatableForRsv(branchRsvBusy, branchRsvBusy2, RS_ENT_BRANCH);
            branchRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_BRANCH, branchRsvIdx2, branchRsvIdx);

            //////// ls index calculation
            auto[lsRsvBusy , lsRsvIdx ] = rsvs.ls.buildFreeIndex(nullptr);
            auto[lsRsvBusy2, lsRsvIdx2] = rsvs.ls.buildFreeIndex(&(lsRsvIdx+1));
            opr& isLsRsvAllocatable = isAlocatableForRsv(lsRsvBusy, lsRsvBusy2, RS_ENT_LDST);
            lsRsvIdx2_final = mux(dcd1(rsEnt) == RS_ENT_LDST, lsRsvIdx2, lsRsvIdx);

            ///// rename command
            RenameCmd renCmd1{dcd1(rdUse)   , regArch.rrf.getReqPtr(),
                              dcd1(rdIdx)   ,
                              dcd1(isBranch), dcd1(specTag)};
            RenameCmd renCmd2{dcd2(rdUse)   , regArch.rrf.getReqPtr()+1,
                              dcd2(rdIdx)   ,
                              dcd2(isBranch), dcd2(specTag)};
            ///// dispatch signal
            opr& isRenamable = regArch.rrf.isRenamable(~dcd2(invalid));
            opr& isdispatable = isAluRsvAllocatable    & isMulRsvAllocatable &
                                isBranchRsvAllocatable & isLsRsvAllocatable &
                                isRenamable;
            ////// pre assign the data to update reservation station
            WireSlot entry1(cvtdecInstrToRsv(dcd1, dcdShare, nullptr        , 0));
            WireSlot entry2(cvtdecInstrToRsv(dcd2, dcdShare, &entry1(rrftag), 1));

            ////// dbg zone
            dbg_isAluRsvAllocatable      = isAluRsvAllocatable;
            dbg_isBranchRsvAllocatable   = isBranchRsvAllocatable;
            dbg_isRenamable              = isRenamable;

            pip(pm.ds.sync){                               initProbe(pipProbGrp .dispatch);
                zyncc(pm.rs.sync, isdispatable){ autoSync  initProbe(zyncProbGrp.dispatch);
                    ////////
                    regArch.rrf.onRename(~dcd1(invalid), ~dcd2(invalid));
                    opr& reqPtr = regArch.rrf.getReqPtr();
                    //////// update arf
                    regArch.arf.onRename(renCmd1, renCmd2);
                    ////// dcd 1 supposed to be valid all the time
                    /***
                     * dispatch entry 1
                     */
                    zif (aluRsvIdx.sl(0)){
                        rsvs.alu2.tryWriteEntry(dcd1(rsEnt),
                                                aluRsvIdx.sl(1, RS_ENT_SEL+1),
                                                entry1);
                    }zelse{
                        rsvs.alu1.tryWriteEntry(dcd1(rsEnt),
                                                aluRsvIdx.sl(1, RS_ENT_SEL+1),
                                                entry1);
                    }
                    rsvs.mul.tryWriteEntry(dcd1(rsEnt), mulRsvIdx   , entry1);
                    rsvs.br .tryWriteEntry(dcd1(rsEnt), branchRsvIdx, entry1);
                    rsvs.ls .tryWriteEntry(dcd1(rsEnt), lsRsvIdx    , entry1);
                    rob.onDispatch(reqPtr, dcd1, dcdShare); //// acknowledge reroder buffer
                    dbg_isDisp1 = 1;
                    /***
                     * dispatch entry 2
                     */
                    zif(~dcd2(invalid)){
                        zif (aluRsvIdx2_final.sl(0)){
                            rsvs.alu2.tryWriteEntry(dcd2(rsEnt),
                                                    aluRsvIdx2_final(1, RS_ENT_SEL+1),
                                                    entry2);
                        }zelse{
                            rsvs.alu1.tryWriteEntry(dcd2(rsEnt),
                                                    aluRsvIdx2_final.sl(1, RS_ENT_SEL+1),
                                                    entry2);
                        }
                        rsvs.mul.tryWriteEntry(dcd2(rsEnt), mulRsvIdx2_final   , entry2);
                        rsvs.br .tryWriteEntry(dcd2(rsEnt), branchRsvIdx2_final, entry2);
                        rsvs.ls .tryWriteEntry(dcd2(rsEnt), lsRsvIdx2_final    , entry2);
                        rob.onDispatch(reqPtr+1, dcd2, dcdShare);
                        dbg_isDisp2 = 1;
                    }
                }
            }
        }
    };
}
#endif //KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

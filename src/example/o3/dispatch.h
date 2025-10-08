//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H
#define KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

#include "kathryn.h"

#include "parameter.h"
#include "immGen.h"
#include "irsv.h"
#include "orsv.h"
#include "srcOpr.h"
#include "rob.h"
#include "stageStruct.h"


namespace kathryn::o3{

    struct DpMod: Module{
        PipStage&    pm;
        ORsv&        aluRsv;
        IRsv&        branchRSV;
        RegArch&     regArch;
        TagMgmt&     tagMgmt;
        Rob&         rob;

        mWire(aluRsvIdx2_final   , ALU_ENT_NUM);
        mWire(branchRsvIdx2_final, BRANCH_ENT_SEL);

        DpMod(PipStage& pm, ORsv& aluRsv, IRsv& branchRSV,
              RegArch& regArch, TagMgmt& tagMgmt, Rob& rob):
            pm         (pm),
            aluRsv     (aluRsv),
            branchRSV  (branchRSV),
            regArch    (regArch),
            tagMgmt    (tagMgmt),
            rob        (rob){}

        Operable& isRsvRequired(RegSlot& dcd, int RS_ENT_IDX){
            return (dcd(rsEnt) == RS_ENT_IDX) & (~dcd(invalid));
        }

        Operable& isAlocatableForRsv(opr& busy1, opr& busy2, int RS_ENT_IDX){
            return  ((~busy1).uext(2) + (~busy2).uext(2)) >=
                (isRsvRequired(pm.dc.dcd1, RS_ENT_IDX).uext(2) +
                 isRsvRequired(pm.dc.dcd2, RS_ENT_IDX).uext(2));
        }

        WireSlot cvtdecInstrToRsv(RegSlot& dcd, RegSlot& dcdShard, int decLaneIdx,
                            opr* desPrevIdx, opr* isDesPrevUse){

            /////// create rsv smRsvI for inorder is redundant
            WireSlot des(smRsvO + smRsvBranch + smRsvBase); /// smRsvBase + smRsvOI
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
            des(rrftag)  = regArch.rrf.getReqPtr() + decLaneIdx;
            des(rdUse)   = dcd(rdUse);
            des(aluOp)   = dcd(aluOp);
            des(spec)    = dcd(spec);
            des(specTag) = dcd(specTag);

            des(phyIdx_1, rsValid_1) = decodeSrcOpr(dcd, desPrevIdx, isDesPrevUse,
                                1, regArch);
            des(phyIdx_2, rsValid_2) = decodeSrcOpr(dcd, desPrevIdx, isDesPrevUse,
                                2, regArch);
            ////////////// branch
            des(opcode)    = dcd(inst)(0, 7);
            des(pred_addr) = dcd(pred_addr);

            return des;
        }

        void flow() override{

            auto& dcd1 = pm.dc.dcd1;
            auto& dcd2 = pm.dc.dcd2;
            auto& dcdShare     = pm.dc.dcdShared;

            auto[aluRsvBusy , aluRsvIdx ] = aluRsv.buildFreeOhIndex(nullptr);
            auto[aluRsvBusy2, aluRsvIdx2] = aluRsv.buildFreeOhIndex(&aluRsvIdx);
            opr& isAluRsvAllocatable = isAlocatableForRsv(aluRsvBusy, aluRsvBusy2, RS_ENT_ALU);

            ///// dcd1 supposed to be correct
            zif (dcd1(rsEnt) == RS_ENT_ALU) aluRsvIdx2_final = aluRsvIdx2.getIdx();
            zelse                           aluRsvIdx2_final = aluRsvIdx.getIdx();


            //// TODO check the assign order
            auto[branchRsvBusy , branchRsvIdx ] = branchRSV.buildFreeBinIndex(nullptr);
            auto[branchRsvBusy2, branchRsvIdx2] = branchRSV.buildFreeBinIndex(&(branchRsvIdx+1));
            opr& isBranchRsvAllocatable = isAlocatableForRsv(branchRsvBusy, branchRsvBusy2, RS_ENT_BRANCH);

            ///// dcd2 supposed to be correct no need to check the valid the exec sequence make ti
            zif (dcd1(rsEnt) == RS_ENT_BRANCH) branchRsvIdx2_final = branchRsvIdx2;
            zelse                              branchRsvIdx2_final = branchRsvIdx;

            ///// rename command
            RenameCmd renCmd1{dcd1(rdUse)   , regArch.rrf.getReqPtr(),
                              dcd1(rdIdx)   ,
                              dcd1(isBranch), dcd1(specTag)};
            RenameCmd renCmd2{dcd2(rdUse)   , regArch.rrf.getReqPtr()+1,
                              dcd2(rdIdx)   ,
                              dcd2(isBranch), dcd2(specTag)};

            opr& isdispatable = (~(isAluRsvAllocatable && isBranchRsvAllocatable)) &
                                regArch.rrf.isRenamable(~dcd2(invalid));


            pip(pm.ds.sync){
                zync_con(isdispatable){
                    ////////
                    regArch.rrf.onRename(~dcd1(invalid), ~dcd2(invalid));
                    opr& reqPtr = regArch.rrf.getReqPtr();
                    //////// update arf
                    regArch.arf.onRename(renCmd1, renCmd2,tagMgmt.mpft);
                    //////// update reservation station
                    WireSlot entry1 = cvtdecInstrToRsv(dcd1, dcdShare, 0, nullptr, nullptr);
                    WireSlot entry2 = cvtdecInstrToRsv(dcd2, dcdShare, 1,
                        &dcd1(rdUse), &dcd1(rdIdx));

                    //////// put the instruction to rob and rsv

                    ////// dcd 1 supposed to be valid all the time
                    zif(dcd1(rsEnt) == RS_ENT_ALU){
                        aluRsv.writeEntry(aluRsvIdx, entry1);
                    }zelse{
                        branchRSV.writeEntry(branchRsvIdx, entry1);
                    }
                    rob.onDispatch(reqPtr, entry1);

                    zif(~dcd2(invalid)){
                        zif(dcd2(rsEnt) == RS_ENT_ALU){
                                aluRsv.writeEntry(OH(aluRsvIdx2_final), entry2);
                        }zelse{
                                branchRSV.writeEntry(branchRsvIdx2_final, entry2);
                        }
                        rob.onDispatch(reqPtr+1, entry2);
                    }
                }
            }
        }


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

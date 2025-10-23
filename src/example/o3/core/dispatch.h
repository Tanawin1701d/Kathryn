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
#include "srcOpr.h"
#include "rob.h"
#include "stageStruct.h"
#include "example/o3/simulation/proberGrp.h"


namespace kathryn::o3{

    struct DpMod: Module{
        PipStage&    pm;
        ORsv&        aluRsv;
        IRsv&        branchRSV;
        RegArch&     regArch;
        TagMgmt&     tagMgmt;
        Rob&         rob;

        mWire(aluRsvIdx2_final   , ALU_ENT_NUM);  //// it is one hot index
        mWire(branchRsvIdx2_final, BRANCH_ENT_SEL); //// it is binary index

        mWire(dbg_isAluRsvAllocatable, 1);
        mWire(dbg_isBranchRsvAllocatable, 1);
        mWire(dbg_isRenamable, 1);

        mWire(dbg_isDisp1, 1);
        mWire(dbg_isDisp2, 1);

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

        WireSlot cvtdecInstrToRsv(RegSlot& dcd, RegSlot& dcdShard, opr* desRrf , int decLaneIdx){
            /////// decLaneIdx start from 0
            /////// create rsv smRsvI for inorder is redundant
            WireSlot des(smRsvO + smRsvBase + smRsvBranch + smRsvAlu); /// smRsvBase + smRsvOI
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

            auto& dcd1     = pm.dc.dcd1;
            auto& dcd2     = pm.dc.dcd2;
            auto& dcdShare = pm.dc.dcdShared;

            auto[aluRsvBusy , aluRsvIdx ] = aluRsv.buildFreeOhIndex(nullptr);
            auto[aluRsvBusy2, aluRsvIdx2] = aluRsv.buildFreeOhIndex(&aluRsvIdx);
            opr& isAluRsvAllocatable = isAlocatableForRsv(aluRsvBusy, aluRsvBusy2, RS_ENT_ALU);

            ///// dcd1 supposed to be correct
            zif (dcd1(rsEnt) == RS_ENT_ALU) aluRsvIdx2_final = aluRsvIdx2.getIdx();
            zelse                           aluRsvIdx2_final = aluRsvIdx.getIdx();

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

            opr& isRenamable = regArch.rrf.isRenamable(~dcd2(invalid));

            opr& isdispatable = isAluRsvAllocatable & isBranchRsvAllocatable & isRenamable;
            ////// pre assign the data
            //////// update reservation station
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
                    zif(dcd1(rsEnt) == RS_ENT_ALU){
                        aluRsv.writeEntry(aluRsvIdx, entry1);
                    }zelse{
                        branchRSV.writeEntry(branchRsvIdx, entry1);
                    }
                    ////// update reorder buffer
                    dbg_isDisp1 = 1;
                    rob.onDispatch(reqPtr, dcd1);

                    zif(~dcd2(invalid)){
                        zif(dcd2(rsEnt) == RS_ENT_ALU){
                                aluRsv.writeEntry(OH(aluRsvIdx2_final), entry2);
                        }zelse{
                                branchRSV.writeEntry(branchRsvIdx2_final, entry2);
                        }
                        rob.onDispatch(reqPtr+1, dcd2);
                        dbg_isDisp2 = 1;
                    }

                }
            }


        }


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

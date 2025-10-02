//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H
#define KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

#include "arf.h"
#include "immGen.h"
#include "irsv.h"
#include "kathryn.h"
#include "orsv.h"
#include "parameter.h"
#include "rrf.h"
#include "srcOpr.h"
#include "stageStruct.h"


namespace kathryn::o3{

    struct DpMod: public Module{
        DecodeStage& dec;
        DispStage    dis;
        ORsv&        aluRsv;
        IRsv&        branchRSV;
        RegArch&     regArch;
        ByPassPool&  bypassPool;

        mWire(aluRsvIdx2_final, ALU_ENT_SEL);
        mWire(branchRsvIdx2_final, BRANCH_ENT_SEL);

        DpMod(DecodeStage& dec, ORsv& aluRsv, IRsv& branchRSV,
              RegArch& regArch, ByPassPool& bp):
            dec        (dec),
            aluRsv     (aluRsv),
            branchRSV  (branchRSV),
            regArch    (regArch),
            bypassPool (bp){}

        Operable& isRsvRequired(RegSlot& dcd, int RS_ENT_IDX){
            return (dcd(rsEnt) == RS_ENT_IDX) & (~dcd(invalid));
        }

        Operable& isAlocatableForRsv(opr& busy1, opr& busy2, int RS_ENT_IDX){
            return  (~busy1 + ~busy2) >=
                (isRsvRequired(dec.dcd1, RS_ENT_IDX) +
                 isRsvRequired(dec.dcd2, RS_ENT_IDX));

        }

        WireSlot cvtdecInstrToRsv(RegSlot& dcd, RegSlot& dcdShard, int decLaneIdx,
                            opr* desPrevIdx, opr* isDesPrevUse){

            /////// create rsv
            WireSlot des(smRsvBase + smRsvO + smRsvI); /// smRsvBase + smRsvOI
                                              /// dcdShared
            des(busy)    = 1;
            if (decLaneIdx == 0){
                des(pc) = dcdShard(pc);
            }else{
                des(pc) = dcdShard(pc) + 4;
            }

            immGen( dcd(inst), dcd(imm_type), des(imm));
            des(rrftag)  = rrf.getReqPtr() + decLaneIdx;
            des(rdUse)   = dcd(rdUse);
            des(aluOp)   = dcd(aluOp);
            des(spec)    = dcd(spec);
            des(specTag) = dcd(specTag);
            des(opcode)  = dcd(inst)(0, 7);
            des(phyIdx_1, rsValid_1) = decodeSrcOpr(dcd, desPrevIdx, isDesPrevUse,
                                1, arf, rrf, bypass);
            des(phyIdx_2, rsValid_2) = decodeSrcOpr(dcd, desPrevIdx, isDesPrevUse,
                                1, arf, rrf, bypass);

            return des;
        }

        void flow() override{

            auto& dcd1 = dec.dcd1;
            auto& dcd2 = dec.dcd2;
            auto& dcdShare     = dec.dcdShared;

            auto[aluRsvBusy , aluRsvIdx ] = aluRsv.buildFreeIndex(nullptr);
            auto[aluRsvBusy2, aluRsvIdx2] = aluRsv.buildFreeIndex(&aluRsvIdx);
            opr& isAluRsvAllocatable = isAlocatableForRsv(aluRsvBusy, aluRsvBusy2, RS_ENT_ALU);

            ///// dcd1 supposed to be correct
            zif (dcd1(rsEnt) == RS_ENT_ALU) aluRsvIdx2_final = aluRsvIdx2.getIdx();
            zelse aluRsvIdx2_final = aluRsvIdx.getIdx();



            //// TODO check the assign order
            auto[branchRsvBusy, branchRsvIdx] = branchRSV.buildFreeIndex(nullptr);
            auto[branchRsvBusy2, branchRsvIdx2] = branchRSV.buildFreeIndex(&branchRsvIdx);
            opr& isBranchRsvAllocatable = isAlocatableForRsv(branchRsvBusy, branchRsvBusy2, RS_ENT_BRANCH);

            ///// dcd2 supposed to be correct no need to check the valid the exec sequence make ti
            zif (dcd1(rsEnt) == RS_ENT_BRANCH) branchRsvIdx2_final = branchRsvIdx2.getIdx();
            zelse branchRsvIdx2_final = branchRsvIdx.getIdx();


            RenameCmd renCmd1{dcd1(rdUse), rrf.getReqPtr(),
                           dcd1(rdIdx),
                            dcd1(isBranch), dcd1(specTag)};
            RenameCmd renCmd2{dcd2(rdUse), rrf.getReqPtr()+1,
                           dcd2(rdIdx),
                            dcd2(isBranch), dcd2(specTag)};


            pip(dis.sync){

                cdowhile( ~(isAluRsvAllocatable && isBranchRsvAllocatable /** and rrf is free */)){
                    //////// update rrf
                    rrf.onRename(~dcd1(invalid), ~dcd2(invalid));
                    //////// update arf
                    arf.onRename(renCmd1, renCmd2);
                    //////// update reservation station
                    WireSlot entry1 = cvtdecInstrToRsv(dcd1, dcdShare, 0, nullptr, nullptr);
                    WireSlot entry2 = cvtdecInstrToRsv(dcd2, dcdShare, 1,
                        &dcd1(rdUse), &dcd1(rdIdx));

                    ////// dcd 1 supposed to be valid all the time
                    zif(dcd1(rsEnt) == RS_ENT_ALU){
                        aluRsv.writeEntry(aluRsvIdx, entry1);
                    }zelse{
                        branchRSV.writeEntry(branchRsvIdx, entry1);
                    }

                    zif(~dcd2(invalid)){
                        zif(dcd2(rsEnt) == RS_ENT_ALU){
                                aluRsv.writeEntry(aluRsvIdx2_final, entry1);
                        }zelse{
                                branchRSV.writeEntry(branchRsvIdx2_final, entry1);
                        }
                    }
                    //////// update commit
                    ///// TODO build the commit


                }

            }

        }


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_DISPATCH_H

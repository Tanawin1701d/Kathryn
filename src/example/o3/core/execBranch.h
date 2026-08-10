//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H
#define KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H

#include "kathryn.h"
#include "alu.h"
#include "rob.h"
#include "rsvs.h"


namespace kathryn::o3{

    struct DpMod;                ///HLH EXEC_BRANCH
    struct RsvBase;              ///HLH EXEC_BRANCH
    struct BranchExec: Module{   ///MD EXEC_BRANCH

        TagMgmt&      tagMgmt;   ///CTRL_HC EXEC_BRANCH
        RegArch&      regArch;   ///CTRL_HC+DATA_HC EXEC_BRANCH
        PipStage&     pm;        ///CTRL_HC+DATA_HC EXEC_BRANCH
        DpMod&        dispMod;   ///CTRL_HC+DATA_HC EXEC_BRANCH
        Rob&          rob;       ///CTRL_HC+DATA_HC EXEC_BRANCH
        ByPass&       bp;        ///CTRL_HC+DATA_HC EXEC_BRANCH
        Rsvs&         rsvs;      ///CTRL_HC+DATA_HC EXEC_BRANCH
        StoreBuf&     stBuf;     ///CTRL_HC+DATA_HC EXEC_BRANCH
        RegSlot&      src;       ///DATA_HC EXEC_BRANCH

        PipSimProbe* psp = nullptr;  ///DC
        mWire(calAddr, ADDR_LEN);   ///DATA_HWD EXEC_BRANCH
        mWire(brTaken, 1);          ///CTRL_HWD EXEC_BRANCH


        explicit BranchExec(TagMgmt&  tagMgmt,   ///CTRL_HC EXEC_BRANCH
                            RegArch&  regArch,   ///CTRL_HC+DATA_HC EXEC_BRANCH
                            PipStage& pm,        ///CTRL_HC+DATA_HC EXEC_BRANCH
                            DpMod&    dispMod,   ///CTRL_HC+DATA_HC EXEC_BRANCH
                            Rob&      rob,       ///CTRL_HC+DATA_HC EXEC_BRANCH
                            StoreBuf& stBuf,     ///CTRL_HC+DATA_HC EXEC_BRANCH
                            Rsvs&     rsvs) :    ///CTRL_HC+DATA_HC EXEC_BRANCH
        tagMgmt (tagMgmt),                       ///CTRL_HC EXEC_BRANCH
        regArch (regArch),                       ///CTRL_HC+DATA_HC EXEC_BRANCH
        pm      (pm),                            ///CTRL_HC+DATA_HC EXEC_BRANCH
        dispMod (dispMod),                       ///CTRL_HC+DATA_HC EXEC_BRANCH
        rob     (rob),                           ///CTRL_HC+DATA_HC EXEC_BRANCH
        stBuf   (stBuf),                         ///CTRL_HC+DATA_HC EXEC_BRANCH
        bp      (regArch.bpp.addByPassEle()),    ///CTRL_HC+DATA_HC EXEC_BRANCH
        rsvs    (rsvs),                          ///CTRL_HC+DATA_HC EXEC_BRANCH
        src     (rsvs.br.execSrc){}              ///DATA_HC EXEC_BRANCH

        void setSimProbe(PipSimProbe* in_psp){psp = in_psp;} ///DC

        void flow() override{   ///HLH EXEC_BRANCH


            //// initialize variable
            opr& opc     = src(opcode);                         ///DATA_DT EXEC_BRANCH
            opr& srcPc   = src(pc);                             ///DATA_DT EXEC_BRANCH
            opr& srcImm  = src(imm_br);                         ///DATA_DT EXEC_BRANCH
            opr& spTag   = src(specTag);                        ///CTRL_DT EXEC_BRANCH
            opr& fixTag  = tagMgmt.mpft.getFixTag(OH(spTag));   ///CTRL_CL EXEC_BRANCH

            opr& srcA   = src(phyIdx_1);                         ///DATA_DT EXEC_BRANCH
            opr& srcB   = src(phyIdx_2); //// take imm from br   ///DATA_DT EXEC_BRANCH
            brTaken     = alu(src(aluOp), srcA, srcB).sl(0);     ///CTRL_CL EXEC_BRANCH

            //// assign static wire to bc
            tagMgmt.bc.sucTag = spTag;    ///CTRL_DT EXEC_BRANCH
            tagMgmt.bc.fixTag = fixTag;   ///CTRL_DT EXEC_BRANCH

            //// calculate the address
            opr& nextPc = srcPc + 4;      ///DATA_CL EXEC_BRANCH
            bp.addSrc(src(rrftag), nextPc); ///// add src for bypass but the bypass trigger is in zync block   ///CTRL_HC+DATA_HC EXEC_BRANCH

            calAddr = nextPc;                                                    ///DATA_DT EXEC_BRANCH
            zif  (opc == RV32_JALR)          calAddr = src(phyIdx_1) + srcImm;   ///DATA_CL EXEC_BRANCH
            zelif((opc==RV32_JAL) | brTaken) calAddr = srcPc + srcImm;           ///DATA_CL EXEC_BRANCH

            opr& brCond = ((opc == RV32_JALR) | (opc==RV32_JAL) | brTaken);      ///CTRL_CL EXEC_BRANCH



            pip(rsvs.br.sync){  tryInitProbe(psp);              ///CTRL_HWD+CTRL_CL EXEC_BRANCH

                /////// write back the data if it needed
                rob.onWriteBack(src(rrftag));                   ///CTRL_HC EXEC_BRANCH
                zif(src(rdUse)){                                ///CTRL_CL EXEC_BRANCH
                    regArch.rrf.onWback(src(rrftag), nextPc);   ///CTRL_HC+DATA_HC EXEC_BRANCH
                    regArch.bpp.doByPass(bp);                   ///CTRL_HC+DATA_HC EXEC_BRANCH
                }

                /////// success predict
                zif (src(pred_addr) == calAddr){ //// case sucPred   ///CTRL_CL EXEC_BRANCH
                    onSucPred(fixTag, src(specTag));   ///CTRL_HC EXEC_BRANCH
                }zelse{ //////// case misPred
                    onMisPred(fixTag, src(specTag), calAddr);   ///CTRL_HC+DATA_HC EXEC_BRANCH
                }
            }
        }

        void onMisPred(opr& fixTag, opr& misTag, opr& fixPc);   ///CTRL_HC+DATA_HC EXEC_BRANCH

        void onSucPred(opr& fixTag, opr& sucTag);   ///CTRL_HC EXEC_BRANCH

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H

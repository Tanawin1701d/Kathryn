//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H
#define KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H

#include "alu.h"
#include "kathryn.h"
#include "parameter.h"
#include "rob.h"
#include "slotParam.h"
#include "stageStruct.h"
#include "isaParam.h"
#include "srcSel.h"
#include "rsvs.h"


namespace kathryn::o3{

    struct FetchMod;
    struct DpMod;
    struct RsvBase;
    struct BranchExec: Module{

        TagMgmt&      tagMgmt;
        RegArch&      regArch;
        PipStage&     pm;
        FetchMod&     fetchMod;
        DpMod&        dispMod;
        Rob&          rob;
        ByPass&       bp;
        Rsvs&         rsvs;
        RegSlot&      src;
        PipSimProbe* psp = nullptr;
        mWire(calAddr, ADDR_LEN);
        mWire(brTaken, 1);


        explicit BranchExec(TagMgmt& tagMgmt,
                            RegArch& regArch,
                            PipStage& pm,
                            FetchMod& fetchMod,
                            DpMod&    dispMod,
                            Rob& rob,
                            Rsvs& rsvs) :
        tagMgmt(tagMgmt),
        regArch(regArch),
        pm(pm),
        fetchMod(fetchMod),
        dispMod(dispMod),
        rob(rob),
        bp(regArch.bpp.addByPassEle()),
        rsvs(rsvs),
        src(rsvs.br.execSrc){}

        void setSimProbe(PipSimProbe* in_psp){psp = in_psp;}

        void flow() override{


            //// initialize variable
            opr& opc     = src(opcode);
            opr& srcPc   = src(pc);
            opr& srcImm  = src(imm_br);
            opr& spTag   = src(specTag);
            opr& fixTag  = tagMgmt.mpft.getFixTag(OH(spTag));

            opr& srcA   = getAluSrcA(src);
            opr& srcB   = getAluSrcB(src, true); //// take imm from br
            brTaken     = alu(src(aluOp), srcA, srcB).sl(0);

            //// assign static wire to bc
            tagMgmt.bc.misTag = spTag;
            tagMgmt.bc.sucTag = spTag;

            //// calculate the address
            opr& nextPc = srcPc + 4;
            bp.addSrc(src(rrftag), nextPc); ///// add src for bypass but the bypass trigger is in zync block

            calAddr = nextPc;
            zif  (opc == RV32_JALR)          calAddr = src(phyIdx_1) + srcImm;
            zelif((opc==RV32_JAL) | brTaken) calAddr = srcPc + srcImm;

            opr& brCond = ((opc == RV32_JALR) | (opc==RV32_JAL) | brTaken);



            pip(pm.br.sync){  tryInitProbe(psp);

                /////// write back the data if it needed
                rob.onWriteBackBranch(src(rrftag), calAddr, brCond);
                zif(src(rdUse)){
                    regArch.rrf.onWback(src(rrftag), nextPc);
                    regArch.bpp.doByPass(bp);
                }

                /////// success predict
                zif (src(pred_addr) == calAddr){ //// case sucPred
                    onSucPred(fixTag, src(specTag));
                }zelse{ //////// case misPred
                    onMisPred(fixTag, src(specTag), calAddr);
                }
            }
        }

        void onMisPred(opr& fixTag, opr& misTag, opr& fixPc);

        void onSucPred(opr& fixTag, opr& sucTag);

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H

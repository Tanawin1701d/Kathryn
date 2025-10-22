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


namespace kathryn::o3{

    struct RsvBase;
    struct BranchExec: Module{

        TagMgmt&      tagMgmt;
        RegArch&      regArch;
        PipStage&     pm;
        Rob&          rob;
        RegSlot&      src;
        ByPass&       bp;
        std::vector<RsvBase*> rsvs;
        PipSimProbe* psp = nullptr;
        mWire(calAddr, ADDR_LEN);
        mWire(brTaken, 1);


        explicit BranchExec(TagMgmt& tagMgmt,
                            RegArch& regArch,
                            PipStage& pm,
                            Rob& rob,
                            RegSlot& src) :
        tagMgmt(tagMgmt),
        regArch(regArch),
        pm(pm),
        rob(rob),
        src(src),
        bp(regArch.bpp.addByPassEle()){}

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
            calAddr = nextPc;
            zif  (opc == RV32_JALR)          calAddr = src(phyIdx_1) + srcImm;
            zelif((opc==RV32_JAL) | brTaken) calAddr = srcPc + srcImm;
            bp.addSrc(src(rrftag), nextPc);


            pip(pm.br.sync){  tryInitProbe(psp);

                /////// write back the data if it needed
                rob.onWriteBack(src(rrftag));
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

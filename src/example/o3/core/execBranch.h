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

    struct DpMod;
    struct RsvBase;
    struct BranchExec: Module{

        TagMgmt&      tagMgmt;
        RegArch&      regArch;
        PipStage&     pm;
        DpMod&        dispMod;
        Rob&          rob;
        ByPass&       bp;
        Rsvs&         rsvs;
        StoreBuf&     stBuf;
        RegSlot&      src;

        PipSimProbe* psp = nullptr;  ///DC
        mWire(calAddr, ADDR_LEN);
        mWire(brTaken, 1);


        explicit BranchExec(TagMgmt& tagMgmt,
                            RegArch& regArch,
                            PipStage& pm,
                            DpMod&    dispMod,
                            Rob& rob,
                            StoreBuf& stBuf,
                            Rsvs& rsvs) :
        tagMgmt(tagMgmt),
        regArch(regArch),
        pm(pm),
        dispMod(dispMod),
        rob(rob),
        stBuf(stBuf),
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

            opr& srcA   = src(phyIdx_1);
            opr& srcB   = src(phyIdx_2); //// take imm from br
            brTaken     = alu(src(aluOp), srcA, srcB).sl(0);

            //// assign static wire to bc
            tagMgmt.bc.sucTag = spTag;
            tagMgmt.bc.fixTag = fixTag;

            //// calculate the address
            opr& nextPc = srcPc + 4;
            bp.addSrc(src(rrftag), nextPc); ///// add src for bypass but the bypass trigger is in zync block

            calAddr = nextPc;
            zif  (opc == RV32_JALR)          calAddr = src(phyIdx_1) + srcImm;
            zelif((opc==RV32_JAL) | brTaken) calAddr = srcPc + srcImm;

            opr& brCond = ((opc == RV32_JALR) | (opc==RV32_JAL) | brTaken);



            pip(rsvs.br.sync){  tryInitProbe(psp); ///CTRL EXEC_BRANCH

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

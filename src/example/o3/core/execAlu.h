//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_ALUEXEC_H
#define SRC_EXAMPLE_O3_ALUEXEC_H

#include "alu.h"
#include "kathryn.h"
#include "rob.h"
#include "srcSel.h"
#include "stageStruct.h"

namespace kathryn::o3{

    struct ExecAlu: Module{
    ExecStage& exSt;
    RegArch&   regArch;
    Rob&       rob;
    RegSlot&   src;
    ByPass&    bp;
    PipSimProbe* psp = nullptr;

    explicit ExecAlu(ExecStage& exSt,
                     RegArch& regArch,
                     Rob& rob,
                     RegSlot& src) :
        exSt(exSt),
        regArch(regArch),
        rob(rob),
        src(src),
        bp(regArch.bpp.addByPassEle()){
        exSt.sync.setTagTracker(src);
    }

    void setSimProbe(PipSimProbe* in_psp){psp = in_psp;}

    void flow() override{

        opr& srcA   = getAluSrcA(src);
        opr& srcB   = getAluSrcB(src);
        opr& result = alu(src(aluOp), srcA, srcB);
        bp.addSrc(src(rrftag), result);

        ///// init pip meta data
        pip(exSt.sync){ tryInitProbe(psp);
            rob.onWriteBack(src(rrftag));
            zif(src(rdUse)){
                regArch.rrf.onWback(src(rrftag), result);
                regArch.bpp.doByPass(bp);
            }
        }
        
    }

    };

}

#endif //SRC_EXAMPLE_O3_ALUEXEC_H
//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_ALUEXEC_H
#define SRC_EXAMPLE_O3_ALUEXEC_H

#include "alu.h"
#include "kathryn.h"
#include "rob.h"
#include "srcSel.h"

namespace kathryn::o3{

    struct ExecAlu: Module{

    ExecStage& exSt;
    RegArch&   regArch;
    RegSlot&   src;
    Rob&       rob;
    ByPass&    bp;


    explicit ExecAlu(ExecStage& exSt,
                     RegArch& regArch,
                     RegSlot& src,
                     Rob& rob) :
        exSt(exSt),
        regArch(regArch),
        src(src),
        rob(rob),
        bp(regArch.bpp.addByPassEle()){}

    void flow() override{

        opr& srcA   = getAluSrcA(src);
        opr& srcB   = getAluSrcB(src);
        opr& result = alu(src(aluOp), srcA, srcB);
        bp.addSrc(src(rrftag), result);

        pip(exSt.sync){
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
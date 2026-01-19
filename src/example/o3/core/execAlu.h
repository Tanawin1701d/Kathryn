//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_ALUEXEC_H
#define SRC_EXAMPLE_O3_ALUEXEC_H


#include "kathryn.h"
#include "alu.h"
#include "rob.h"
#include "srcSel.h"
#include "stageStruct.h"
#include "rsv.h"

namespace kathryn::o3{

    struct ExecAlu: Module{
    RegArch&     regArch;
    Rob&         rob;
    RsvBase&     rsv;
    ByPass&      bp;
    PipSimProbe* psp = nullptr; ///DC

    explicit ExecAlu(RegArch& regArch,
                     Rob& rob,
                     RsvBase& rsvBase) :
        regArch(regArch),
        rob(rob),
        rsv(rsvBase),
        bp(regArch.bpp.addByPassEle()){
        // exSync.setTagTracker(src);
    }

    void setSimProbe(PipSimProbe* in_psp){psp = in_psp;}

    void flow() override{

        RegSlot& src    = rsv.execSrc;
        opr&     srcA   = getAluSrcA(src);
        opr&     srcB   = getAluSrcB(src);
        opr&     result = alu(src(aluOp), srcA, srcB);
        bp.addSrc(src(rrftag), result);

        ///// init pip meta data
        pip(rsv.sync){ tryInitProbe(psp);
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
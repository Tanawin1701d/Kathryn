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

    struct AluExec: Module{

    RegSlot src;
    SyncMeta syncMeta;
    Rob&    rob;
    Rrf&    rrf;

    explicit AluExec(RegSlot srcData, Rob& rob, Rrf& rrf):
    src(srcData),
    rob(rob),
    rrf(rrf){}

    void flow() override{

        opr& srcA   = getAluSrcA(src);
        opr& srcB   = getAluSrcB(src);
        opr& result = alu(src(aluOp), srcA, srcB);

        pip(syncMeta){
            rob.onWriteBack(src(rrftag));
            rrf.onWback(src(rrftag), result);
            /////// do the bypass
        }
        
    }

    }

}

#endif //SRC_EXAMPLE_O3_ALUEXEC_H
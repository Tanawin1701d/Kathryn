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

    explicit AluExec(const RegSlot& srcData, const SyncMeta& syncMeta, Rob& rob, Rrf& rrf) :
        src(srcData),
        syncMeta(syncMeta),
        rob(rob),
        rrf(rrf){}

    void flow() override{

        opr& srcA   = getAluSrcA(src);
        opr& srcB   = getAluSrcB(src);
        opr& result = alu(src(aluOp), srcA, srcB);

        pip(syncMeta){
            rob.onWriteBack(src(rrftag));
            zif(src(rdUse)){
                rrf.onWback(src(rrftag), result);
            }

            /////// do the bypass and misspredict handler
        }
        
    }

    }

}

#endif //SRC_EXAMPLE_O3_ALUEXEC_H
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
    RegSlot    src;

    explicit ExecAlu(ExecStage& exSt, RegArch& regArch) :
        exSt(exSt), regArch(regArch){}

    void flow() override{

        opr& srcA   = getAluSrcA(*src);
        opr& srcB   = getAluSrcB(*src);
        opr& result = alu(*src(aluOp), srcA, srcB);

        pip(exSt.sync){
            regArch.rob.onWriteBack(src(rrftag));
            zif(src(rdUse)){
                regArch.rrf.onWback(src(rrftag), result);
            }

            /////// do the bypass and misspredict handler
        }
        
    }

    };

}

#endif //SRC_EXAMPLE_O3_ALUEXEC_H
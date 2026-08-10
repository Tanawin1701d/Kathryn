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

    struct ExecAlu: Module{      ///MD EXEC_ALU
    RegArch&     regArch;        ///CTRL_HC+DATA_HC EXEC_ALU
    Rob&         rob;            ///CTRL_HC+DATA_HC EXEC_ALU
    RsvBase&     rsv;            ///CTRL_HC+DATA_HC EXEC_ALU
    ByPass&      bp;             ///CTRL_HC+DATA_HC EXEC_ALU
    PipSimProbe* psp = nullptr; ///DC

    explicit ExecAlu(RegArch& regArch,     ///CTRL_HC+DATA_HC EXEC_ALU
                     Rob& rob,             ///CTRL_HC+DATA_HC EXEC_ALU
                     RsvBase& rsvBase) :   ///CTRL_HC+DATA_HC EXEC_ALU
        regArch(regArch),                  ///CTRL_HC+DATA_HC EXEC_ALU
        rob(rob),                          ///CTRL_HC+DATA_HC EXEC_ALU
        rsv(rsvBase),                      ///CTRL_HC+DATA_HC EXEC_ALU
        bp(regArch.bpp.addByPassEle()){    ///CTRL_HC+DATA_HC EXEC_ALU
        // exSync.setTagTracker(src);
    }

    void setSimProbe(PipSimProbe* in_psp){psp = in_psp;} ///DC

    void flow() override{   ///HLH EXEC_ALU

        RegSlot& src    = rsv.execSrc;       ///CTRL_DT+DATA_DT EXEC_ALU
        opr&     srcA   = getAluSrcA(src);   ///DATA_CL EXEC_ALU
        opr&     srcB   = getAluSrcB(src);   ///DATA_CL EXEC_ALU
        opr&     result = alu(src(aluOp), srcA, srcB);   ///DATA_CL EXEC_ALU
        bp.addSrc(src(rrftag), result);   ///CTRL_HC+DATA_HC EXEC_ALU

        ///// init pip meta data
        pip(rsv.sync){ tryInitProbe(psp);   ///CTRL_HWD+CTRL_CL EXEC_ALU
            rob.onWriteBack(src(rrftag));   ///CTRL_HC EXEC_ALU
            zif(src(rdUse)){   ///CTRL_CL EXEC_ALU
                regArch.rrf.onWback(src(rrftag), result);   ///CTRL_HC+DATA_HC EXEC_ALU
                regArch.bpp.doByPass(bp);                   ///CTRL_HC+DATA_HC EXEC_ALU
            }
        }

    }

    };

}

#endif //SRC_EXAMPLE_O3_ALUEXEC_H

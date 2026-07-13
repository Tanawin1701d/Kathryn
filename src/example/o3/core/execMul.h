//
// Created by tanawin on 11/12/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H
#define KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H

#include "kathryn.h"
#include "multiplier.h"
#include "rob.h"
#include "stageStruct.h"

namespace kathryn::o3{


    struct ExecMul: Module{   ///MD EXEC_MUL
        RegArch&   regArch;   ///CTRL_HC+DATA_HC EXEC_MUL
        Rob&       rob;       ///CTRL_HC+DATA_HC EXEC_MUL
        RsvBase&   rsv;       ///CTRL_HC+DATA_HC EXEC_MUL
        ByPass&    bp;        ///CTRL_HC+DATA_HC EXEC_MUL
        PipSimProbe* psp = nullptr; ///DC

        explicit ExecMul(RegArch& regArch,     ///CTRL_HC+DATA_HC EXEC_MUL
                         Rob&     rob,         ///CTRL_HC+DATA_HC EXEC_MUL
                         RsvBase& rsv          ///CTRL_HC+DATA_HC EXEC_MUL
                         ):
        regArch(regArch),                      ///CTRL_HC+DATA_HC EXEC_MUL
        rob    (rob),                          ///CTRL_HC+DATA_HC EXEC_MUL
        rsv    (rsv),                          ///CTRL_HC+DATA_HC EXEC_MUL
        bp     (regArch.bpp.addByPassEle()){   ///CTRL_HC+DATA_HC EXEC_MUL
            //mulSync.setTagTracker(src);
        }

        void setSimProbe(PipSimProbe* in_psp){psp = in_psp;} ///DC

        void flow() override{   ///HLH EXEC_MUL

            RegSlot& src    = rsv.execSrc;   ///CTRL_DT+DATA_DT EXEC_MUL
            opr& mulRes = multiplier(src);   ///DATA_CL EXEC_MUL
            bp.addSrc(src(rrftag), mulRes);  ///CTRL_HC+DATA_HC EXEC_MUL

            pip(rsv.sync){ tryInitProbe(psp);   ///CTRL_HWD+CTRL_CL EXEC_MUL
                rob.onWriteBack(src(rrftag));   ///CTRL_HC EXEC_MUL
                zif(src(rdUse)){   ///CTRL_CL EXEC_MUL
                    regArch.rrf.onWback(src(rrftag), mulRes);   ///CTRL_HC+DATA_HC EXEC_MUL
                    regArch.bpp.doByPass(bp);   ///CTRL_HC+DATA_HC EXEC_MUL
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H

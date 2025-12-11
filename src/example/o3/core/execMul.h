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


    struct ExecMul: Module{
        ExecStage&  mulSt;
        RegArch&   regArch;
        Rob&       rob;
        RegSlot&   src;
        ByPass&    bp;
        PipSimProbe* psp = nullptr;

        explicit ExecMul(ExecStage& muSt,
                         RegArch& regArch,
                         Rob& rob,
                         RegSlot& src):
        mulSt(muSt),
        regArch(regArch),
        rob(rob),
        src(src),
        bp(regArch.bpp.addByPassEle()){
            mulSt.sync.setTagTracker(src);
        }

        void setSimProbe(PipSimProbe* in_psp){psp = in_psp;}

        void flow() override{

            opr& mulRes = multiplier(src);
            bp.addSrc(src(rrftag), mulRes);

            pip(mulSt.sync){tryInitProbe(psp);
                rob.onWriteBack(src(rrftag));
                zif(src(rdUse)){
                    regArch.rrf.onWback(src(rrftag), mulRes);
                    regArch.bpp.doByPass(bp);
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H
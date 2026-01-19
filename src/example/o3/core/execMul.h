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
        RegArch&   regArch;
        Rob&       rob;
        RsvBase&   rsv;
        ByPass&    bp;
        PipSimProbe* psp = nullptr; ///DC

        explicit ExecMul(RegArch& regArch,
                         Rob&     rob,
                         RsvBase& rsv
                         ):
        regArch(regArch),
        rob    (rob),
        rsv    (rsv),
        bp     (regArch.bpp.addByPassEle()){
            //mulSync.setTagTracker(src);
        }

        void setSimProbe(PipSimProbe* in_psp){psp = in_psp;}

        void flow() override{

            RegSlot& src    = rsv.execSrc;
            opr& mulRes = multiplier(src);
            bp.addSrc(src(rrftag), mulRes);

            pip(rsv.sync){ tryInitProbe(psp);
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
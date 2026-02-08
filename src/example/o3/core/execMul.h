//
// Created by tanawin on 11/12/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H
#define KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H

#include "kathryn.h"
#include "multiplier.h"
#include "rob.h"
#include "stageStruct.h"
#include "lib/math/math.h"

namespace kathryn::o3{


    struct ExecMul: Module{
        RegArch&   regArch;
        Rob&       rob;
        RsvBase&   rsv;
        ByPass&    bp;
        bool       isSqrt;
        mReg(r1 , 32);
        mReg(r2 , 32);
        mReg(res, 32);
        PipSimProbe* psp = nullptr; ///DC

        explicit ExecMul(RegArch& regArch,
                         Rob&     rob,
                         RsvBase& rsv,
                         bool     isSqrt
                         ):
        regArch(regArch),
        rob    (rob),
        rsv    (rsv),
        bp     (regArch.bpp.addByPassEle()),
        isSqrt (isSqrt){
            //mulSync.setTagTracker(src);
        }

        void setSimProbe(PipSimProbe* in_psp){psp = in_psp;}

        void flow() override{

            if (isSqrt){
                mWire(mulRes, 32);
                mulRes = r1 * r2;
                RegSlot& src   = rsv.execSrc;
                bp.addSrc(src(rrftag), mulRes);
                pip(rsv.sync){ tryInitProbe(psp);
                    seq{
                        par{
                            sqrtInt(src(phyIdx_1), r1);
                            sqrtInt(src(phyIdx_2), r2);
                        }
                        par{
                            rob.onWriteBack(src(rrftag));
                            zif(src(rdUse)){
                                regArch.rrf.onWback(src(rrftag), mulRes);
                                regArch.bpp.doByPass(bp);
                            }
                        }
                    }
                }
            }else{
                RegSlot& src    = rsv.execSrc;
                opr& mulRes = multiplier(src);
                bp.addSrc(src(rrftag), mulRes);

                pip(rsv.sync){ tryInitProbe(psp); ///CTRL EXEC_MUL
                    rob.onWriteBack(src(rrftag));
                    zif(src(rdUse)){
                        regArch.rrf.onWback(src(rrftag), mulRes);
                        regArch.bpp.doByPass(bp);
                    }
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H
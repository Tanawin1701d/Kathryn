//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_SRCSEL_H
#define SRC_EXAMPLE_O3_SRCSEL_H

#include "parameter.h"

namespace kathryn::o3{

    inline opr& getAluSrcA(RegSlot& src){             ///DATA_HC SHARED_COMP
        mWire(out, DATA_LEN);                         ///DATA_HWD SHARED_COMP
        opr& srcSel = src(rsSel_1);                   ///DATA_DT SHARED_COMP
        ztate(srcSel){                                ///DATA_CL SHARED_COMP
            zcase(SRC_A_RS1) {out = src(phyIdx_1);}   ///DATA_DT SHARED_COMP
            zcase(SRC_A_PC ) {out = src(pc);      }   ///DATA_DT SHARED_COMP
            zcasedef         {out = 0;            }   ///DATA_DT SHARED_COMP
        }
        return out;   ///DATA_HC SHARED_COMP
    }

    inline opr& getAluSrcB(RegSlot& src, bool isBr = false){                 ///DATA_HC SHARED_COMP
        mWire(out, DATA_LEN);                                                ///DATA_HWD SHARED_COMP
        opr& srcSel = src(rsSel_2);                                          ///DATA_DT SHARED_COMP
        ztate(srcSel){                                                       ///DATA_CL SHARED_COMP
             zcase   (SRC_B_RS2 ) { out = src(phyIdx_2);                 }   ///DATA_DT SHARED_COMP
             zcase   (SRC_B_IMM ) { out = (isBr? src(imm_br): src(imm)); }   ///DATA_CL SHARED_COMP
             zcase   (SRC_B_FOUR) { out = 4;                             }   ///DATA_DT SHARED_COMP
             zcasedef             { out = 0;                             }   ///DATA_DT SHARED_COMP
        }
        return out;                                                          ///DATA_HC SHARED_COMP
    }

}

#endif //SRC_EXAMPLE_O3_SRCSEL_H

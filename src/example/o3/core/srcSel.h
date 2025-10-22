//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_SRCSEL_H
#define SRC_EXAMPLE_O3_SRCSEL_H

#include "kathryn.h"
#include "rob.h"

namespace kathryn::o3{

    inline opr& getAluSrcA(RegSlot& src){
        mWire(out, DATA_LEN);
        opr& srcSel = src(rsSel_1);
        zif  (srcSel == SRC_A_RS1) out = src(phyIdx_1);
        zelif(srcSel == SRC_A_PC ) out = src(pc);
        zelse                      out = 0;
        return out;
    }

    inline opr& getAluSrcB(RegSlot& src, bool isBr = false){
        mWire(out, DATA_LEN);
        opr& srcSel = src(rsSel_2);
        zif  (srcSel == SRC_B_RS2 ) out = src(phyIdx_2);
        zelif(srcSel == SRC_B_IMM ) out = (isBr? src(imm_br): src(imm));
        zelif(srcSel == SRC_B_FOUR) out = 4;
        zelse                       out = 0;
        return out;
    }

}

#endif //SRC_EXAMPLE_O3_SRCSEL_H

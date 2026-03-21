//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_SRCSEL_H
#define SRC_EXAMPLE_O3_SRCSEL_H

#include "parameter.h"

namespace kathryn::o3{

    inline opr& get_alu_src_a(RegSlot& src){
        m_wire(out, DATA_LEN);
        opr& src_sel = src(rsSel_1);
        ztate(src_sel){
            zcase(SRC_A_RS1) {out = src(phyIdx_1);}
            zcase(SRC_A_PC ) {out = src(pc);      }
            zcasedef         {out = 0;            }
        }
        return out;
    }

    inline opr& get_alu_src_b(RegSlot& src, bool is_br = false){
        m_wire(out, DATA_LEN);
        opr& src_sel = src(rsSel_2);
        ztate(src_sel){
             zcase   (SRC_B_RS2 ) { out = src(phyIdx_2);                 }
             zcase   (SRC_B_IMM ) { out = (is_br? src(imm_br): src(imm)); }
             zcase   (SRC_B_FOUR) { out = 4;                             }
             zcasedef             { out = 0;                             }
        }
        return out;
    }

}

#endif //SRC_EXAMPLE_O3_SRCSEL_H

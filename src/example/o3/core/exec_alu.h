//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_ALUEXEC_H
#define SRC_EXAMPLE_O3_ALUEXEC_H


#include "kathryn.h"
#include "alu.h"
#include "rob.h"
#include "src_sel.h"
#include "stage_struct.h"
#include "rsv.h"

namespace kathryn::o3{

    struct ExecAlu: Module{
    RegArch&     reg_arch;
    Rob&         rob;
    RsvBase&     rsv;
    ByPass&      bp;
    PipSimProbe* psp = nullptr; ///DC

    explicit ExecAlu(RegArch& reg_arch,
                     Rob& rob,
                     RsvBase& rsv_base) :
        reg_arch(reg_arch),
        rob(rob),
        rsv(rsv_base),
        bp(reg_arch.bpp.add_by_pass_ele()){
        // ex_sync.set_tag_tracker(src);
    }

    void set_sim_probe(PipSimProbe* in_psp){psp = in_psp;}

    void flow() override{

        RegSlot& src    = rsv.exec_src;
        opr&     src_a   = get_alu_src_a(src);
        opr&     src_b   = get_alu_src_b(src);
        opr&     result = alu(src(alu_op), src_a, src_b);
        bp.add_src(src(rrftag), result);

        ///// init pip meta data
        pip(rsv.sync){ try_init_probe(psp); ///CTRL EXEC_ALU
            rob.on_write_back(src(rrftag));
            zif(src(rd_use)){
                reg_arch.rrf.on_wback(src(rrftag), result);
                reg_arch.bpp.do_by_pass(bp);
            }
        }
        
    }

    };

}

#endif //SRC_EXAMPLE_O3_ALUEXEC_H
//
// Created by tanawin on 11/12/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H
#define KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H

#include "kathryn.h"
#include "multiplier.h"
#include "rob.h"
#include "stage_struct.h"

namespace kathryn::o3{


    struct ExecMul: Module{
        RegArch&   reg_arch;
        Rob&       rob;
        RsvBase&   rsv;
        ByPass&    bp;
        PipSimProbe* psp = nullptr; ///DC

        explicit ExecMul(RegArch& reg_arch,
                         Rob&     rob,
                         RsvBase& rsv
                         ):
        reg_arch(reg_arch),
        rob    (rob),
        rsv    (rsv),
        bp     (reg_arch.bpp.add_by_pass_ele()){
            //mul_sync.set_tag_tracker(src);
        }

        void set_sim_probe(PipSimProbe* in_psp){psp = in_psp;}

        void flow() override{

            RegSlot& src    = rsv.exec_src;
            opr& mul_res = multiplier(src);
            bp.add_src(src(rrftag), mul_res);

            pip(rsv.sync){ try_init_probe(psp); ///CTRL EXEC_MUL
                rob.on_write_back(src(rrftag));
                zif(src(rd_use)){
                    reg_arch.rrf.on_wback(src(rrftag), mul_res);
                    reg_arch.bpp.do_by_pass(bp);
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_CORE_EXECMUL_H
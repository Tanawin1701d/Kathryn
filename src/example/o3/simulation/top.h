//
// Created by tanawin on 19/12/25.
//

#ifndef EXAMPLE_O3_SIMULATION_TOP_H
#define EXAMPLE_O3_SIMULATION_TOP_H

#include "kathryn.h"
#include "../core/core.h"

namespace kathryn::o3{

    struct TopSim: Module{

        m_mod(my_core, Core, 0);

        ///// imem
        m_reg(ij_imem0, INST_WIDTH);
        m_reg(ij_imem1, INST_WIDTH);
        m_reg(ij_imem2, INST_WIDTH);
        m_reg(ij_imem3, INST_WIDTH);

        ///// dmem
        m_reg(ij_dmem0, DATA_LEN);

        TopSim(int x){};

        void flow() override{
            ///// imem
            my_core.pm.ft.i_mem0 = ij_imem0;
            my_core.pm.ft.i_mem1 = ij_imem1;
            my_core.pm.ft.i_mem2 = ij_imem2;
            my_core.pm.ft.i_mem3 = ij_imem3;
            ///// dmem
            my_core.pm.ld_st.dmem_rdata = ij_dmem0;
        }

    };

}

#endif //EXAMPLE_O3_CORE_TOP_H
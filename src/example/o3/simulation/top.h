//
// Created by tanawin on 19/12/25.
//

#ifndef EXAMPLE_O3_SIMULATION_TOP_H
#define EXAMPLE_O3_SIMULATION_TOP_H

#include "kathryn.h"
#include "../core/core.h"

namespace kathryn::o3{

    struct TopSim: Module{

        mMod(myCore, Core, 0);

        mReg(ijImem0, INST_WIDTH);
        mReg(ijImem1, INST_WIDTH);
        mReg(ijImem2, INST_WIDTH);
        mReg(ijImem3, INST_WIDTH);

        TopSim(int x){};

        void flow() override{
            myCore.pm.ft.iMem0 = ijImem0;
            myCore.pm.ft.iMem1 = ijImem1;
            myCore.pm.ft.iMem2 = ijImem2;
            myCore.pm.ft.iMem3 = ijImem3;
        }

    };

}

#endif //EXAMPLE_O3_CORE_TOP_H
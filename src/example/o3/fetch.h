//
// Created by tanawin on 5/10/25.
//

#ifndef KATHRYN_FETCH_H
#define KATHRYN_FETCH_H

#include "kathryn.h"
#include "stageStruct.h"
#include "parameter.h"

namespace kathryn::o3{

    struct FetchMod : Module{
        PipStage& pm;

        mWire(pc, ADDR_LEN);

        explicit FetchMod(PipStage&  pm){}

        void flow() override{


            pip(pm.ft.sync){ autoStart

                zync(pm.dc.sync){
                    pc <<= pc + 8;

                }

            }

        }
    };
}

#endif //KATHRYN_FETCH_H
//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H
#define KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    struct SyncPip: SyncMeta{

        expression spectag {SPECTAG_LEN};

        void killSlave(opr& killTag){ //// kill Tag is one hot for all system
            mWire(slaveKiller, 1);
            slaveKiller = 1;
            ///////// TODO pool signal

        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

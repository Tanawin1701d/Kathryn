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

        SyncPip(const std::string& name):
        SyncMeta(name){}

        void killSlave(opr& killTag){ //// kill Tag is one hot for all system
            mWire(slaveKiller, 1);
            slaveKiller = (killTag & spectag) != 0;
            ///////// TODO pool signal

        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

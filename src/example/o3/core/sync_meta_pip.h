//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H
#define KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

#include "parameter.h"

namespace kathryn::o3{

    struct SyncPip: SyncMeta{

        RegSlot* src = nullptr;

        SyncPip(const std::string& name):
        SyncMeta(name){}

        void kill_if_tag_met(bool auto_restart, opr& fix_tag){ //// kill Tag is one hot for all system

            opr& pip_kill_condition = ( (*src)(spec) & (((*src)(spec_tag) & fix_tag) != 0));
            kill_slave(auto_restart, &pip_kill_condition);
        }

        void set_tag_tracker(RegSlot& ref_src){
            src = &ref_src;
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

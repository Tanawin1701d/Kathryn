//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H
#define KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    struct SyncPip: SyncMeta{

        RegSlot* src = nullptr;

        SyncPip(const std::string& name):
        SyncMeta(name){}

        void killIfTagMet(bool autoRestart, opr& fixTag){ //// kill Tag is one hot for all system

            opr& pipKillCondition = ( (*src)(spec) & (((*src)(specTag) & fixTag) != 0));
            killSlave(autoRestart, &pipKillCondition);
        }

        void setTagTracker(RegSlot& refSrc){
            src = &refSrc;
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

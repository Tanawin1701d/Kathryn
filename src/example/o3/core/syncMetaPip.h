//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H
#define KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

#include "parameter.h"

namespace kathryn::o3{

    struct SyncPip: SyncMeta{   ///MD SHARED_COMP

        RegSlot* src = nullptr;   ///CTRL_HC SHARED_COMP

        SyncPip(const std::string& name):   ///HLH SHARED_COMP
        SyncMeta(name){}                    ///HLH SHARED_COMP

        void killIfTagMet(bool autoRestart, opr& fixTag){ //// kill Tag is one hot for all system   ///CTRL_HC SHARED_COMP

            opr& pipKillCondition = ( (*src)(spec) & (((*src)(specTag) & fixTag) != 0));            ///CTRL_CL SHARED_COMP
            killSlave(autoRestart, &pipKillCondition);                                              ///CTRL_CL SHARED_COMP
        }

        void setTagTracker(RegSlot& refSrc){   ///DATA_HC SHARED_COMP
            src = &refSrc;   ///CTRL_DT SHARED_COMP
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_SYNCMETAPIP_H

//
// Created by tanawin on 18/1/2567.
//

#include "modelSimInterface.h"

namespace kathryn {
    void FlowSimulatable::afterSimCollectData() {
        assert(_engine != nullptr);
        _engine->finalizeData();
    }


}
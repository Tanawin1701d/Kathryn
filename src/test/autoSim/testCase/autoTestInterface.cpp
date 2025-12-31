//
// Created by tanawin on 7/2/2567.
//


#include "autoTestInterface.h"
#include "test/autoSim/simMng.h"

namespace kathryn{


    AutoTestEle::AutoTestEle(int simId): _simId(simId) {
        addSimTestToPool(this);
    }
}
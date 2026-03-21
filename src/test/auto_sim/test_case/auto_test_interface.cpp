//
// Created by tanawin on 7/2/2567.
//


#include "auto_test_interface.h"
#include "test/auto_sim/sim_mng.h"

namespace kathryn{


    AutoTestEle::AutoTestEle(int sim_id): _simId(sim_id) {
        add_sim_test_to_pool(this);
    }
}
//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_SIMMNG_H
#define KATHRYN_SIMMNG_H

#include "vector"
#include "unordered_set"

#include "kathryn.h"
#include "sim_auto_interface.h"
#include "test_case/auto_test_interface.h"
#include "util/term_color/term_color.h"

namespace kathryn{


    void add_sim_test_to_pool(AutoTestEle* sim_ele);

    void start_auto_sim_test(PARAM& param);

}

#endif //KATHRYN_SIMMNG_H

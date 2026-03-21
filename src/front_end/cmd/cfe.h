//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_CFE_H
#define KATHRYN_CFE_H

#include "iostream"
#include "vector"
#include "string"

#include "util/term_color/term_color.h"


#include "param_reader.h"


namespace kathryn {

    void print_welcome_screen();

    void test_simple(PARAM& params);

    void start(PARAM& params);

    void test_riscv(PARAM& params);

    void test_riscv_sort(PARAM& params);

    void test_riscv_gen(PARAM& params);

    void test_o3_sim(PARAM& params);

    void test_kride_sim(PARAM& params);

    void test_ride_sim(PARAM& params);

    void test_comb_kride_ride(PARAM& params);

    void test_o3_gen(PARAM& params);
}

#endif //KATHRYN_CFE_H

//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_CFE_H
#define KATHRYN_CFE_H

#include <iostream>
#include <vector>
#include <string>

#include "util/termColor/termColor.h"

#include "example/riscv/simulation/RISCV_sim.h"


namespace kathryn {

    void printWelcomeScreen();

    void test(int idx,
              std::vector<std::string>& args);

    void start(std::vector<std::string>& args);

    void test_riscv();

}

#endif //KATHRYN_CFE_H

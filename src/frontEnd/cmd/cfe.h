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
#include "paramReader.h"


namespace kathryn {

    void printWelcomeScreen();

    void test_simple(PARAM& params);

    void start(std::string paramFilePath);

    void test_riscv(PARAM& params);

}

#endif //KATHRYN_CFE_H

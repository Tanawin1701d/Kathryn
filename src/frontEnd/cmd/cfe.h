//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_CFE_H
#define KATHRYN_CFE_H

#include "iostream"
#include "vector"
#include "string"

#include "util/termColor/termColor.h"


#include "paramReader.h"


namespace kathryn {

    void printWelcomeScreen();

    void testSimple(PARAM& params);

    void start(PARAM& params);

    void testRiscv(PARAM& params);

    void testRiscvSort(PARAM& params);

    void testRiscvGen(PARAM& params);

    void testO3Sim(PARAM& params);

    void testKrideSim(PARAM& params);

    void testRideSim(PARAM& params);

    void testCombKrideRide(PARAM& params);

    void testO3Gen(PARAM& params);
}

#endif //KATHRYN_CFE_H

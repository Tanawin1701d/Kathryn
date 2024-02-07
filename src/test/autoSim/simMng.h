//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_SIMMNG_H
#define KATHRYN_SIMMNG_H

#include<vector>
#include<unordered_set>

#include "kathryn.h"
#include"simAutoInterface.h"
#include"testCase/autoTestInterface.h"
#include "util/termColor/termColor.h"

namespace kathryn{


    void addSimTestToPool(AutoTestEle* simEle);

    void startAutoSimTest();

}

#endif //KATHRYN_SIMMNG_H

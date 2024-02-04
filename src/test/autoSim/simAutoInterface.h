//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_SIMAUTOINTERFACE_H
#define KATHRYN_SIMAUTOINTERFACE_H

#include "sim/interface/simInterface.h"

namespace kathryn{




    class SimAutoInterface: public SimInterface{

    public:

        explicit SimAutoInterface(CYCLE limitCycle, std::string vcdFilePath);


    };

}

#endif //KATHRYN_SIMAUTOINTERFACE_H

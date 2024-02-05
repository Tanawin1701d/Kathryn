//
// Created by tanawin on 4/2/2567.
//


#include "simAutoInterface.h"
#include "simMng.h"

namespace kathryn{


    SimAutoInterface::SimAutoInterface(int simId,
                                       CYCLE limitCycle,
                                       std::string vcdFilePath) :
    SimInterface(limitCycle,std::move(vcdFilePath)),
    _simId(simId){

        addSimTestToPool(this);

    }

}
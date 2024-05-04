//
// Created by tanawin on 4/2/2567.
//


#include "simAutoInterface.h"

#include <utility>
#include "simMng.h"

namespace kathryn{


    SimAutoInterface::SimAutoInterface(int simId,
                                       CYCLE limitCycle,
                                       std::string vcdFilePath,
                                       std::string profileFilePath
                                       ) :
    SimInterface(limitCycle,
                 std::move(vcdFilePath),
                 std::move(profileFilePath)),
    _simId(simId){
    }




    }

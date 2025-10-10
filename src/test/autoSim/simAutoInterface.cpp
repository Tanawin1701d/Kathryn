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
                                       std::string profileFilePath,
                                       SimProxyBuildMode simProxyBuildMode,
                                       bool reqInline,
                                       int  opLevel,
                                       const std::string& genPref
                                       ) :
    SimInterface(limitCycle,
                 std::move(vcdFilePath),
                 std::move(profileFilePath),
                 "simple_gen_file_" + genPref + "_" + std::to_string(simId),
                 simProxyBuildMode,
                 false,
                 reqInline,
                 opLevel),
    _simId(simId){
    }




    }

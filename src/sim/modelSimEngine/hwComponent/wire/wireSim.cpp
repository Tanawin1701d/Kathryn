//
// Created by tanawin on 18/7/2024.
//

#include "wireSim.h"



namespace kathryn{


    /**
         *
         *   wire sim engine
         *
         ***/
    WireSimEngine::WireSimEngine(Wire* master, VCD_SIG_TYPE sigType):
    LogicSimEngine(
        (Assignable*  ) master,
        (Identifiable*) master,
        VST_WIRE,
        false,
        0),
    _master(master){
        assert(_master != nullptr);
    }



}
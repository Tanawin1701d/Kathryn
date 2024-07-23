//
// Created by tanawin on 18/7/2024.
//

#include "wireSim.h"

#include <model/hwComponent/wire/wire.h>


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
        ///std::cout << "wire simengine is created" << std::endl;
        //std::cout << master->getVarName() << std::endl;
        assert(_master != nullptr);
    }



}
